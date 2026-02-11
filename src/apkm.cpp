#include "apkm.hpp"
#include <sys/system_properties.h>
#include <unistd.h>
#include <sys/stat.h>
#include "idiomas.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <queue>
#include <dlfcn.h>
#include <sstream>
#include <iostream>
#include <sys/ioctl.h> // Para ioctl e TIOCGWINSZ
#include <unistd.h>    // Para STDOUT_FILENO

using json = nlohmann::json;
using namespace std;
using namespace Utilitarios;

//Construtor básico para adição de um novo repositório
RepoConfig::RepoConfig(bool toAdd, std::string url){
    this->toAdd=toAdd;
    this->url=url;
}

//Construtor padrão
RepoConfig::RepoConfig()
{

};

//Construtor que recebe vetor de strings com os dados do pacote
DadosPacote::DadosPacote(string raizrepo, vector<string> dados){
    this->pacote=dados[0];
    this->nome=dados[1];
    this->descrição=dados[2];
    this->versão=dados[3];
    this->sha256sum=dados[4];
    string enderecoCompleto = dados[5];
    //Caso contenha https:// é um link externo, caso contrário é um diretório do servidor a partir do raizrepo
    if(!enderecoCompleto.empty()){
        if((enderecoCompleto.find("https://") != std::string::npos) || raizrepo==""){
            this->endereço=enderecoCompleto;
        }else{
            this->endereço=raizrepo+enderecoCompleto;
        }
    }else{
        this->endereço="ERRO_NO_ADDRESS_PROVIDED";
    }
    cout << "JSONARCHS: " << dados[6] << endl;

}

DadosPacote* DadosPacote::fromJson(string jsonstr){
    DadosPacote* pacote = new DadosPacote();
    try{
        json j = json::parse(jsonstr);
        cout << jsonstr << endl;
        if(j.contains("pacote")) j.at("pacote").get_to(pacote->pacote);
        if(j.contains("nome")) j.at("nome").get_to(pacote->nome);
        if(j.contains("descricao")) j.at("descricao").get_to(pacote->descrição);
        if(j.contains("versao")) j.at("versao").get_to(pacote->versão);
        if(j.contains("sha256sumOrPGPLink")) j.at("sha256sumOrPGPLink").get_to(pacote->sha256sum);
        if(j.contains("endereco")) j.at("endereco").get_to(pacote->endereço);
        if(j.contains("arquiteturas")) j.at("arquiteturas").get_to(pacote->arquiteturas);      
    }catch(...){
        return nullptr;
    }
    return pacote;
}

DadosPacote::DadosPacote(){
    //Construtor padrão, implementar lógica se necessário.
}

//Converte string JSON para estrutura RemoteRepoConfig (Contem os dados obtidos do servidor/Addon)
RemoteRepoConfig* RemoteRepoConfig::fromJson(std::string jsonstring) {
    try {
        // Tenta fazer o parse. Se falhar, pula para o 'catch'
        json j = json::parse(jsonstring);
        RemoteRepoConfig* r = new RemoteRepoConfig();

        if(j.contains("name")) j.at("name").get_to(r->name);
        if(j.contains("repository_sources_path")) j.at("repository_sources_path").get_to(r->repository_sources_path);

        if (j.contains("packages") && j["packages"].is_array()) {
            for (const auto& item : j["packages"]) {
                if (item.is_array()) {
                    // Aqui você chama o seu construtor de DadosPacote
                    DadosPacote* pacDados = new DadosPacote(r->repository_sources_path, item.get<std::vector<std::string>>());
                    pacDados->origem=r->name+" (Repo)";
                    r->pacotes.push_back(pacDados);
                }
            }
        }
        return r;
    } catch (...) {
        //Retorna nulo em caso de falha no parse
        return nullptr;
    }
}

RemoteRepoConfig* RemoteRepoConfig::fromJsonOfAddOn(std::string jsonstring){
    try {
        // Tenta fazer o parse. Se falhar, pula para o 'catch'
        json j = json::parse(jsonstring);
        RemoteRepoConfig* r = new RemoteRepoConfig();
        if(j.contains("name")) j.at("name").get_to(r->name);
        if(j.contains("repository_sources_path")) j.at("repository_sources_path").get_to(r->repository_sources_path);

        if (j.contains("packages") && j["packages"].is_array()) {
            for (const auto& item : j["packages"]) {
                // Aqui você chama o seu construtor de DadosPacote
                DadosPacote* pacDados = DadosPacote::fromJson(item.dump());
                pacDados->origem=r->name+" (Repo AddOn)";
                r->pacotes.push_back(pacDados);
            }
        }
        if (j.contains("pinned_hashs") && j["pinned_hashs"].is_array()) {
            j.at("pinned_hashs").get_to(r->pinned_hashes);
        }
        return r;
    } catch (...) {
        //Retorna nulo em caso de falha no parse
    }
    return nullptr;
}

//Converte string JSON para estrutura local RepoConfig
RepoConfig* RepoConfig::from_json(string jsonstr) {
    json j = json::parse(jsonstr);
    RepoConfig* r = new RepoConfig();
    // Mapeamento simples
    j.at("repo_name").get_to(r->name);
    j.at("url").get_to(r->url);

    // Mapeamento de campo aninhado (security -> allowed_hashes)
    if (j.contains("security") && j["security"].contains("allowed_hashes")) {
        j.at("security").at("allowed_hashes").get_to(r->pinned_hashes);
    }
    return r;
}

//Converte os dados da estrutura RepoConfig para string JSON
string RepoConfig::to_json(RepoConfig& r) {
    json j = json{
        {"repo_name", r.name},
        {"url", r.url},
        {"security", {
            {"allowed_hashes", r.pinned_hashes},
            {"check_expiry", true} // Valor padrão
        }}
    };
    return j.dump(4);
}

//Converte os dados da estrutura RepoConfig para string JSON sem argumentos
string RepoConfig::to_json() {
    RepoConfig r = *this;
    json j = json{
        {"repo_name", r.name},
        {"url", r.url},
        {"security", {
            {"allowed_hashes", r.pinned_hashes},
            {"check_expiry", true} // Valor padrão
        }}
    };
    return j.dump(4);
}

//Prepara as configurações da execução para o processamento da ordem
Config::Config(int argc, char* argv[]){
    //Cria os diretórios necessários caso não existam
    if(!filesystem::exists(diretórioDados)){
        filesystem::create_directory(diretórioDados);
    }
    if(!filesystem::exists(diretórioSources)){
        filesystem::create_directory(diretórioSources);
    }
    if(!filesystem::exists(diretórioAddOns)){
        filesystem::create_directory(diretórioAddOns);
    }
    //Prepara o suporte de idioma do sistema
    stringsidioma = new Strings(this);
    //prepara a instância do CURL para ser utilizada
    curl = curl_easy_init(); //<-- Curl iniciado aqui
    //Passa o comando utilizado para invocar o programa
    nomebinario = argv[0];
    //Percorre e processa a lista de argumentos
    for(int arg=1; arg < argc; arg++){
        string argstr = argv[arg];  
        //1 = atualizar (update)
        //2 = adicionar repositório (--add-repo)
        //3 = remover repositórios inválidos (rm-repo)
        //4 = listar repositórios ou addons (--list)
        //5 = listar addons
        //6 = pesquisar pacote (search)
        //7 = instalar pacote (install)
        //8 = desinstalar pacote (uninstall)

        //Desativa SSL temporáriamente se o argumento --no-ssl for encontrado
        if(ssl && argstr=="--no-ssl"){
            ssl=false;
        }

        //Define a opção para assumir "sim" em todas as perguntas
        if(!assumirSim && (argstr=="--yes" || argstr=="-y")){
            assumirSim=true;
        }

        //Mostra pacotes incompatíveis
        if(argstr=="--incompatíveis"){
            exibirIncompatíveis=true;
        }

        //Swaitch acionado apos identificar o comando principal (Diferente de zero)
        switch(instrução){
            case 1:
            {
                //Nenhum argumento adicional esperado
            }
            break;
            case 2:
            {
                url=argstr;
            }
            break;
            case 4:
            {//A seleção entre listar repositórios ou addons acontece aqui
                if(argstr=="repos")
                {
                    instrução=4;
                }else if(argstr=="addons"){
                    instrução=5;
                }
            }
            break;
            case 3:case 6: case 7: case 8:
            {
                nomes.push_back(argstr);
            }
            break;
            //Primeira passagem, identifica o comando
            default:
            {
                if(argstr=="update"){
                    instrução=1;
                }else if(argstr=="--add-repo"){
                    instrução=2;
                }else if(argstr=="--rm-repo"){
                    instrução=3;
                }else if(argstr=="list"){ //Inclue opção para addons e repositórios
                    instrução=4;
                }else if(argstr=="search"){
                    instrução=6;
                }else if(argstr=="install"){
                    instrução=7;
                }else if(argstr=="uninstall"){
                    instrução=8;
                }else{
                    comandoInvalido=argstr;
                }
            }
        }
    }
}

Config::~Config() {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
    for(AddOn* addon : addonsdinamicos) delete(addon);
    for(RemoteRepoConfig* config : reposglobais) {
        for(DadosPacote* pacote : config->pacotes) {
            delete(pacote);
        }
        delete(config);
    }
}

//Mostra as configurações da execução
void Config::printcfg(Config* config, Strings* stringsidioma){
    NLIND(stringsidioma->USANDO_SSL[0]+": "+boolToHLang(config->ssl, stringsidioma));
}

//Converte true para sim e false para não para facilitar o entendimento
string Config::boolToHLang(bool opt, Strings* stringsidioma){
    if(opt){
        return stringsidioma->SIM[0];
    }else{
        return stringsidioma->NAO[0];
    }
}

//Construtor para struct tools
Tools::Tools(std::vector<std::string> certificadoHashs, std::string url, Config* configs, bool usarSSL){
    this->certificadoHashs=certificadoHashs;
    this->url=url;
    this->configs=configs;
    this->usarSSL=usarSSL;
}

//Caminho até a biblioteca .so do AddOn
AddOn::AddOn(std::string addonpacote, Config* mainCfg) {
    this->config = new AddOnConfig();
    this->config->addonpacote=addonpacote;
    this->config->addonpacote=addonpacote;
}

//Destrutor que limpa a instância
AddOn::~AddOn(){
    delete(config);
}

//Carrega todos os AddOns disponíveis na pasta de AddOns
vector<AddOn*> AddOn::CarregarTodos(Config* config){
    vector<AddOn*> addons = std::vector<AddOn*>();
    string saida = executarComandoShell("pm list packages | grep "+config->pacoteAddOns);
    stringReplace(&saida,"package:","");
    vector<string> addonspacks = stringSplit(&saida, '\n');
    for(string addonpack : addonspacks){
        AddOn* addon = new AddOn(addonpack, config);
        if(addon->getConfig()){
            addons.push_back(addon);
        }else{
            delete(addon);
        }
    }
    return addons;
}

//Carrega a configuração do AddOn a partir da instância da biblioteca dinâmica (Determina se é estático ou dinâmico)
bool AddOn::getConfig(){
    vector<string> addonconfig = Query("getConfig");
    try{
        json j = json::parse(addonconfig[0]);
        if(j.contains("descricao")) j.at("descricao").get_to(config->descrição);
        if(j.contains("fornecedor")) j.at("fornecedor").get_to(config->fornecedor);
        if(j.contains("dinamico")) j.at("dinamico").get_to(config->dinamico);
        if(j.contains("versao")) j.at("versao").get_to(config->versão);
        if(j.contains("nomeExibicao")) j.at("nomeExibicao").get_to(config->nome);
        if(j.contains("updateUrl") && j["updateUrl"]!="") j.at("updateUrl").get_to(config->novaversao);
        if(j.contains("prefix") && j["prefix"]!="") j.at("prefix").get_to(config->prefix);
    }catch(...){
        return false;
    }
    return true;
}

void AddOn::exibirAddOnInfos(Config* globalconfig){
    //"ADDON_INFOS": ["Exibindo informações do AddOn: ", "Nome: ","Fornecedor: ", "Dinâmico: ","Descrição: ", "versão: ", "Atualizado: "]
    vector<string>* ADDON_INFOS = &globalconfig->stringsidioma->ADDON_INFOS;
    NLIND((*ADDON_INFOS)[0]);
    cout << gerarLinhaSeparadora() << endl;
    NLINDINFO((*ADDON_INFOS)[1]);
    NLINDINFO((*ADDON_INFOS)[1]+config->nome);
    NLINDINFO((*ADDON_INFOS)[2]+config->fornecedor);
    NLINDINFO((*ADDON_INFOS)[3]+globalconfig->boolToHLang(config->dinamico, globalconfig->stringsidioma));
    NLINDINFO((*ADDON_INFOS)[4]+config->descrição);
    NLINDINFO((*ADDON_INFOS)[5]+config->versão);
    NLINDINFO((*ADDON_INFOS)[6]);
    if(config->novaversao==""){
        cout << globalconfig->stringsidioma->SIM[0];
    }else{
        cout << globalconfig->stringsidioma->NAO[0];
    }
    cout << endl << gerarLinhaSeparadora() << endl;
}

std::vector<DadosPacote*> AddOn::Buscar(string pesquisa){
    std::vector<DadosPacote*> pacotes;
    cout << "Resultados de "+config->nome+": " << endl;
    vector<string> query = Query("search="+pesquisa);
    for(string jsonstr : query){
        DadosPacote* pacote = DadosPacote::fromJson(jsonstr);
        pacote->origem=config->nome+" (Dinamic AddOn)";
        if(pacote){
            pacotes.push_back(pacote);
        }
    }
    return pacotes;
}

RemoteRepoConfig* AddOn::ObterPacote(string pacotestr){
    vector<string> query = Query("getPackage="+pacotestr);
    string jsonstr = query[0];
    RemoteRepoConfig* repo = RemoteRepoConfig::fromJsonOfAddOn(jsonstr);
    if(repo){
        return repo;
    }
    return nullptr;
}

//Utilizado quando um AddOn é do tipo estático (apenas gera o RemoteRepoConfig)
std::vector<RemoteRepoConfig*> AddOn::getRepos(){
    std::vector<RemoteRepoConfig*> repositórios;
    vector<string> query = Query("getRepos");
    for(string jsonstr : query){
        RemoteRepoConfig* repo = RemoteRepoConfig::fromJsonOfAddOn(jsonstr);
        if(repo){
            repositórios.push_back(repo);
        }
    }
    return repositórios;
}

//Efetua a query para o AddOn
vector<string> AddOn::Query(std::string requisição){
    std::string querybruta = Utilitarios::executarComandoShell("content query --uri content://"+config->addonpacote+"/"+requisição);
    vector<string> retornoTmp = Utilitarios::stringSplit(&querybruta, '\n');
    vector<string> retorno;
    for(string rtemp1 : retornoTmp){
        retorno.push_back(Utilitarios::limparSaidaContent(rtemp1));
    }
    return retorno;
}

namespace Utilitarios {

    //Equivalente ao getprop nativo do Android
    std::string getProp(std::string prop) {
        char value[PROP_VALUE_MAX];
        
        // Tenta obter a localidade atual do sistema
        if (__system_property_get(prop.c_str(), value) > 0) {
            return std::string(value);
        }

        return "none";
    }

    //Substitui todas as ocorrências de uma substring por outra em uma string
    void stringReplace(std::string* string, std::string alvo, std::string substituto){
        size_t pos = 0;
        // Loop encontra e substitui até find retornar npos
        while ((pos = string->find(alvo, pos)) != std::string::npos) {
            string->replace(pos, alvo.length(), substituto);
            pos += substituto.length();
        }
    }

    vector<string> stringSplit(std::string* str, char alvo){
        vector<string> arrayfinal;
        std::stringstream ss(*str);
        std::string item;
        while (std::getline(ss, item, alvo)) {
            arrayfinal.push_back(item);
        }
        return arrayfinal;
    }

    bool terminalColor(){
        return isatty(STDOUT_FILENO);
    }

    //Função geral de busca de termo dentro de uma string (case insensitive)
    bool search_match(std::string fonte, std::string termo) {
        // 1. Converte a fonte e o termo para minúsculo para busca insensível
        std::transform(fonte.begin(), fonte.end(), fonte.begin(), ::tolower);
        std::transform(termo.begin(), termo.end(), termo.begin(), ::tolower);

        // 2. Verifica se o termo existe dentro da fonte
        if (fonte.find(termo) != std::string::npos) {
            return true;
        }
        
        return false;
    }

    //Obtém a pasta temporária para uso do apkm
    std::string obterPastaTemporaria() {
        // 1. Tenta /data/local/tmp (Padrão Android Shell)
        struct stat info;
        if (stat("/data/local/tmp", &info) == 0 && (info.st_mode & S_IWOTH || info.st_mode & S_IWUSR)) {
            string apkmtmp = "/data/local/tmp/apkm-tmp";
            if(!filesystem::exists(apkmtmp)){
                filesystem::create_directories(apkmtmp);
            }
            return apkmtmp;
        }

        // 2. Fallback para a pasta atual do binário
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            string apkmtmp = std::string(cwd) + "/apkm-tmp";
            if(!filesystem::exists(apkmtmp)){
                filesystem::create_directories(apkmtmp);
            }
            return apkmtmp;
        }

        // 3. Último recurso: /tmp (Indisponível em alguns dispositivos Android)
        string apkmtmp = "/tmp/apkm-tmp";
        if(!filesystem::exists(apkmtmp)){
            if(!filesystem::create_directories(apkmtmp)){
                return "";
            }
        }
        return apkmtmp;
    }

    //Execulta um comando via Shell nativo
    std::string executarComandoShell(std::string cmd) {
        std::array<char, 128> buffer;
        std::string resultado;
        // Abre um pipe para ler a saída do comando
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) return "ERRO";
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            resultado += buffer.data();
        }
        return resultado;
    }

    std::string limparSaidaContent(std::string bruto) {
        std::string chave = "json_data=";
        size_t pos = bruto.find(chave);
        
        if (pos == std::string::npos) return "[]";

        // O JSON começa logo após o '='
        std::string jsonResult = bruto.substr(pos + chave.length());

        // 1. Remove espaços em branco e quebras de linha do início e do fim
        const std::string whitespace = " \n\r\t";
        size_t first = jsonResult.find_first_not_of(whitespace);
        if (std::string::npos == first) return "[]";
        
        size_t last = jsonResult.find_last_not_of(whitespace);
        return jsonResult.substr(first, (last - first + 1));
    }

    bool checarCompatibilidade(vector<string> abisPacote){
        if(abisPacote[0]=="all") return true;
        string abis = getProp("ro.product.cpu.abilist");
        vector<string> abilista = stringSplit(&abis, ',');
        for(string abichk : abilista)
        for(string abi : abisPacote)
        if(abichk==abi) return true;

        return false;
    }

    int obterLarguraTerminal() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
    }

    std::string gerarLinhaSeparadora() {
        int largura = obterLarguraTerminal();
        if (largura <= 0) largura = 40; // Fallback caso ioctl falhe
        // Cria uma string com 'largura' vezes o caractere definido
        return std::string(largura, LINHASEPARADORACHAR); 
    }

    void NLIND(std::string msg){
        if(terminalColor()){
            CNLIND(msg);
        }else{
            NCNLIND(msg);
        }
    }
    void NLINDERR(std::string msg){
        if(terminalColor()){
            CNLINDERR(msg);
        }else{
            NCNLINDERR(msg);
        }
    }
    void NLINDINFO(std::string msg){
        if(terminalColor()){
            CNLINDINFO(msg);
        }else{
            NCNLINDINFO(msg);
        }
    }

    void NLINDINPUT(std::string msg){
        if(terminalColor()){
            CNLINDINPUT(msg);
        }else{
            NCNLINDINPUT(msg);
        }
    }
}