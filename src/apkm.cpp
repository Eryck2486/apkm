//Este arquivo é parte do projeto APKM, um gerenciador de pacotes para Android com gerenciador root Magisk/KernelSU

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
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include "apkm_packages_manager.hpp"

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
    arquiteturas=stringSplit(&dados[6], ',');
}

//Construtor padrão para json
DadosPacote* DadosPacote::fromJson(string jsonstr){
    DadosPacote* pacote = new DadosPacote();
    try{
        json j = json::parse(jsonstr);
        if(j.contains("pacote")) j.at("pacote").get_to(pacote->pacote);
        if(j.contains("nome")) j.at("nome").get_to(pacote->nome);
        if(j.contains("descricao")) j.at("descricao").get_to(pacote->descrição);
        if(j.contains("versao")) j.at("versao").get_to(pacote->versão);
        if(j.contains("versionCode")) j.at("versionCode").get_to(pacote->versionCode);
        if(j.contains("sha256sumOrPGPLink")) j.at("sha256sumOrPGPLink").get_to(pacote->sha256sum);
        if(j.contains("endereco")) j.at("endereco").get_to(pacote->endereço);
        if(j.contains("arquiteturas") && j["arquiteturas"].is_array()) j.at("arquiteturas").get_to(pacote->arquiteturas);  
        if(j.contains("icon")) j.at("icon").get_to(pacote->icon);
    }catch(...){
        return nullptr;
    }
    return pacote;
}

DadosPacote::DadosPacote(){
    //Construtor padrão, implementar lógica se necessário.
}

string DadosPacote::toJson(){
    json j = json{
        {"pacote", pacote},
        {"nome", nome},
        {"descricao", descrição},
        {"versao", versão},
        {"sha256sumOrPGPLink", sha256sum},
        {"endereco", endereço},
        {"arquiteturas", arquiteturas},
        {"icon", icon},
        {"source", origem}
    };
    string jf = j.dump(4);
    stringReplace(&jf, "\n", ""); // Remover quebras de linha para evitar problemas de formatação
    stringReplace(&jf, "    ", "");
    return jf;
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
        r->origem=r->name+" (Repo)";
        return r;
    } catch (...) {
        //Retorna nulo em caso de falha no parse
        return nullptr;
    }
}

RemoteRepoConfig* RemoteRepoConfig::fromJsonOfAddOn(std::string jsonstring, std::string addonpacote){
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
                pacDados->origem=r->name+" (Repo AddOn) "+addonpacote;
                r->pacotes.push_back(pacDados);
            }
        }
        if (j.contains("pinned_hashs") && j["pinned_hashs"].is_array()) {
            j.at("pinned_hashs").get_to(r->pinned_hashes);
        }
        r->origem=r->name+" (Repo AddOn) "+addonpacote;
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

        
        //Verifica se o argumento é um comando de configuração, caso contrário segue para o switch
        if(!argParam(argstr)) 
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
                }else if(argstr=="--list"){ //Inclue opção para addons e repositórios
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

bool Config::argParam(string argstr){
    //Desativa SSL temporáriamente se o argumento --no-ssl for encontrado
    if(ssl && argstr=="--no-ssl"){
        ssl=false;
        return true;
    }

    //Define a opção para assumir "sim" em todas as perguntas
    if(!assumirSim && (argstr=="--yes" || argstr=="-y")){
        assumirSim=true;
        return true;
    }

    //Define a saída para o formato JSON
    if(argstr=="--json" || argstr=="-j"){
        formatoJSON=true;
        return true;
    }

    //Mostra pacotes incompatíveis
    if(argstr=="--all-archs"){
        exibirIncompatíveis=true;
        return true;
    }
    return false;
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
    if(config->formatoJSON) return;
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
    this->mainCfg=mainCfg;
}

//Destrutor que limpa a instância
AddOn::~AddOn(){
    //Envia um sinal para limpar cache e dados temporários do app.
    ContentQuery("cleanAll");
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
        int tentativas = 5;
        int delay = 5; // 5 segundos
        bool key = false;
        for(int i=0; (i<tentativas && !key); i++){
            if(addon->getConfig()){
                addons.push_back(addon);
                key=true;
            }else{
                addon->startAddOn();
                sleep(delay);
            }
        }
        if(!key) delete(addon);
    }
    return addons;
}

//Inicia o processo de busca de atualizações dos pacotes instalados
std::vector<PackageInfo*> AddOn::getPackagesUpdatesFromJSON(std::string jsonstr){
    vector<PackageInfo*> updates;
    string jsonstrLimpa = jsonstr;
    stringReplace(&jsonstrLimpa, "\n", "&nl"); // Remove quebras de linha para evitar problemas de formatação
    string response = Call("getUpdates="+jsonstrLimpa, true);
    vector<string> dadosPacotes = stringSplit(&response, '\n');
    for(string pacoteJson : dadosPacotes){
        if(pacoteJson!="" && pacoteJson.length()>5){
            try{
                //{"package":"com.karaoke.play","appName":"KARAOKE PLAY","vCode":2,"vName":"3.0"}
                json j = json::parse(pacoteJson);
                string package = j["package"];
                string appName = j["appName"];
                long versionCode = j["vCode"];
                string versionName = j["vName"];
                PackageInfo* pacoteInfo = new PackageInfo(package, appName, versionName, versionCode);
                updates.push_back(pacoteInfo);
            }
            catch(exception e)
            {
                std::cerr << "Erro ao processar pacote: " << pacoteJson << std::endl;
                std::cerr << "Exceção: " << e.what() << std::endl;
            }
        }
    }
    return updates;
}

//Carrega a configuração do AddOn a partir da instância da biblioteca dinâmica (Determina se é estático ou dinâmico)
bool AddOn::getConfig(){
    vector<string> addonconfig = ContentQuery("getConfig");
    try{
        json j = json::parse(addonconfig[0]);
        //Print do JSON para depuração (Remover depois)
        if(j.contains("descricao")) j.at("descricao").get_to(config->descrição);
        if(j.contains("fornecedor")) j.at("fornecedor").get_to(config->fornecedor);
        if(j.contains("dinamico")) j.at("dinamico").get_to(config->dinamico);
        if(j.contains("versao")) j.at("versao").get_to(config->versão);
        if(j.contains("nomeExibicao")) j.at("nomeExibicao").get_to(config->nome);
        if(j.contains("hasUpdate")) j.at("hasUpdate").get_to(config->novaversao);
        if(j.contains("prefix") && j["prefix"]!="") j.at("prefix").get_to(config->prefix);
        if(j.contains("SocketName") && j["SocketName"]!="") j.at("SocketName").get_to(config->socketName);
    }catch(...){
        return false;
    }
    if(config->socketName=="UKNOWN"){
        return false;
    }
    return true;
}

void AddOn::exibirAddOnInfos(Config* globalconfig){
    //"ADDON_INFOS": ["Exibindo informações do AddOn: ", "Nome: ","Fornecedor: ", "Dinâmico: ","Descrição: ", "versão: ", "Atualizado: "]
    vector<string>* ADDON_INFOS = &globalconfig->stringsidioma->ADDON_INFOS;
    NLIND((*ADDON_INFOS)[0]);
    cout << gerarLinhaSeparadora() << endl;
    NLINDINFO((*ADDON_INFOS)[1]+config->nome);
    NLINDINFO((*ADDON_INFOS)[2]+config->fornecedor);
    NLINDINFO((*ADDON_INFOS)[3]+globalconfig->boolToHLang(config->dinamico, globalconfig->stringsidioma));
    NLINDINFO((*ADDON_INFOS)[4]+config->descrição);
    NLINDINFO((*ADDON_INFOS)[5]+config->versão);
    string info = Config::boolToHLang(!config->novaversao, globalconfig->stringsidioma);
    NLINDINFO((*ADDON_INFOS)[6]+info);

    cout << endl << gerarLinhaSeparadora() << endl;
}

std::vector<DadosPacote*> AddOn::Buscar(string pesquisa){
    std::vector<DadosPacote*> pacotes = std::vector<DadosPacote*>();
    string response = Call("search="+pesquisa, true);
    if(response==""){
        return pacotes;
    }
    vector<string> query = stringSplit(&response, '\n');
    for(string jsonstr : query){
        DadosPacote* pacote = DadosPacote::fromJson(jsonstr);
        pacote->origem=config->nome+" (Dinamic AddOn)";
        if(pacote){
            pacotes.push_back(pacote);
        }
    }
    return pacotes;
}

//Solicita um pacote para o Addon e recebe {"status":"status", "packageFile":"/data/user/0/com.apkm.addon.exemplo/cache/arquivo.apk", "package":"com.pacote.app"} ou {"status":"fail", "packageFile":"", "package":""} se o pacote não existe.
//status é "success" para pacote válido e obtido e "fail" para pacote não encontrado.
//deve retornar [arquivo.apk, com.pacote.exemplo] diretório do apk e o id do pacote.
vector<string> AddOn::getPackage(string pacotestr){
    string request = "getPackage="+pacotestr;
    return initDownloadComunication(request);
}

//Utilizado quando um AddOn é do tipo estático (apenas gera o RemoteRepoConfig)
std::vector<RemoteRepoConfig*> AddOn::getRepos(){
    std::vector<RemoteRepoConfig*> repositórios;
    vector<string> query = ContentQuery("getRepos");
    for(string jsonstr : query){
        RemoteRepoConfig* repo = RemoteRepoConfig::fromJsonOfAddOn(jsonstr, config->addonpacote);
        if(repo){
            repositórios.push_back(repo);
        }
    }
    return repositórios;
}

//Efetua a query para o AddOn (Query via ContentProvider)
vector<string> AddOn::ContentQuery(std::string requisição){
    std::string querybruta = Utilitarios::executarComandoShell("content query --uri content://"+config->addonpacote+"/"+requisição);
    vector<string> retornoTmp = Utilitarios::stringSplit(&querybruta, '\n');
    vector<string> retorno;
    for(string rtemp1 : retornoTmp){
        retorno.push_back(Utilitarios::limparSaidaContent(rtemp1));
    }
    return retorno;
}

//Solicita um pacote de atualização do AddOn (self-update), recebe {"status":"success/fail", "packageFile":"/data/user/0/com.apkm.addon.exemplo/cache/arquivo.apk", "package":"com.pacote.exemplo"} ou {"status":"fail", "packageFile":"", "package":""} se o pacote não existe ou ocorreu um erro.
vector<string> AddOn::getAddonUpdate(){
    string rquest = "getUpdate";
    return initDownloadComunication(rquest);
}

//envia uma requisição via socket para o AddOn e aguarda a resposta caso aguardarResposta seja true, caso contrário apenas envia a requisição sem aguardar resposta (utilizado para requisições que não necessitam de resposta como iniciar o serviço do AddOn)
string AddOn::Call(string request, bool aguardarResposta){
    return Utilitarios::requisiçãoViaSocket(config->socketName, request, aguardarResposta);
}

//Função que inicializa o serviço do AddOn caso esteja parado, retorna true se o serviço foi iniciado ou já estava em execução e false caso tenha falhado ao iniciar o serviço.
bool AddOn::startAddOn(){
    string result = executarComandoShell("am start-foreground-service -n "+config->addonpacote+"/.ServiceWorker");
    if(search_match(result, "done")){
        return true;
    }
    return false;
}

//Trava a tarefa atual, exibe a progressão da tarefa do serviço externo e libera a Thread principal já com o resultado
bool AddOn::iniciarSocketFeedback(bool &key){
    bool result = false;
    //Criando linha de feedback
    cout << endl; //Essa linha é substituida pelo indicador de progressão da função barraProgresso
    while (key)
    {
        int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        const char* socket_name = mainCfg->socketName.c_str();
        std::strncpy(&addr.sun_path[1], socket_name, strlen(socket_name));
        // O comprimento deve incluir apenas os bytes usados
        int len = offsetof(struct sockaddr_un, sun_path) + strlen(socket_name) + 1;
        if (::bind(server_fd, (struct sockaddr*)&addr, len) == -1) {
            perror("bind feedback failed");
            return false;
        }
        if (listen(server_fd, 10) < 0) {
            perror("listen failed");
            return false;
        }
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept failed");
            return false;
        }
        char buffer[1024] = {0};
        size_t bytes_read;
        while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0 && key) {
            buffer[bytes_read] = '\0';
            string dadosJSON = buffer;
            if(mainCfg->formatoJSON){
                cout << dadosJSON << endl;
            }else if(stringContains(&dadosJSON, "DOWNLOADING")){
                string porcentagemStr = "0";
                int porcentagem;
                string message = ""; //Texto do que está acontecendo
                string size = ""; //Tamanho do arquivo (KB/MB/GB/etc..)
                //JSON para parse:
                //"DOWNLOADING" : { "percent" : "0", "message" : "Baixando com.termux.apk", "size" : "109,60 MB" }
                stringReplace(&dadosJSON, "\"DOWNLOADING\":", "");
                try{
                    json j = json::parse(dadosJSON);
                    if(j.contains("percent")) j.at("percent").get_to(porcentagemStr);
                    if(j.contains("message")) j.at("message").get_to(message);
                    if(j.contains("size")) j.at("size").get_to(size);
                    porcentagem = atoi(porcentagemStr.c_str());
                }catch(...){
                }
                barraProgresso(porcentagem, message+" "+size);
            }
            if(stringContains(&dadosJSON, "END")){
                string status;
                string message;
                //JSON para parse:
                //"END":{"status":"sucess/fail", "message":"Terminou com sucesso"}
                stringReplace(&dadosJSON, "\"END\":", "");
                try{
                    json j = json::parse(dadosJSON);
                    if(j.contains("status")) j.at("status").get_to(status);
                    if(j.contains("message")) j.at("message").get_to(message);
                }catch(...){
                    key=false;
                }
                if(status=="checking" && !mainCfg->formatoJSON){
                    cout << "\033[A\33[2K\r";
                    cout.flush();
                    NLINDINFO(message);
                }else if(status=="success"){
                    result=true;
                    if(!mainCfg->formatoJSON) NLIND(message);
                    key=false;
                }else if(status=="fail"){
                    result=false;
                    if(!mainCfg->formatoJSON) NLINDERR(message);
                    key=false;
                }
            }
        }
        
        string dadosJSON = buffer;
        close(client_fd);
        close(server_fd);
        result = true;
    }
    return result;
}

//Exibe uma barra de progresso na linha atual do terminal, a barra é atualizada a cada nova chamada da função, o texto é exibido ao lado da barra e a porcentagem é exibida ao final da barra.
void AddOn::barraProgresso(int porcentagem, string texto){
    try{
        int larguraTerminal = obterLarguraTerminal();
        int tamanhoPorcentagemInd = 4;
        char barra = '[';
        char final = ']';
        char indicador = '#';
        char espaco = ' ';
        int tamanhoTextMenssagem = texto.length();
        int tamanhoBarra = larguraTerminal - tamanhoTextMenssagem - tamanhoPorcentagemInd - 3;
        string carregamento;
        string carregamentorestante;
        //Evita divisão por 0 que causa excessão em strings
        if(porcentagem>0){
            float tamanhopedaço = tamanhoBarra/100.0f;
            string carregamentoTmp(int(porcentagem*tamanhopedaço)+1, indicador);
            carregamento = carregamentoTmp;
            int restante = tamanhoBarra - carregamento.length();
            string carregamentorestanteTmp(restante, espaco);
            carregamentorestante = carregamentorestanteTmp;
        }else{
            carregamento = "";
            string carregamentorestanteTmp(tamanhoBarra, espaco);
            carregamentorestante = carregamentorestanteTmp;
        }
        //Removendo linha atual
        std::cout << "\033[A\33[2K\r";
        std::cout.flush();
        //Exibindo progressão
        string resultado = texto + espaco + barra + carregamento+carregamentorestante+final+espaco+to_string(porcentagem)+"%";
        cout << resultado << endl;
    }catch(...){
    }
}

vector<string> AddOn::initDownloadComunication(string request){
    vector<string> resultado = std::vector<string>();
    //Inicia call em thread
    string response;
    //Cria uma thread para o socket principal
    bool ciclo = true;
    std::thread processo = std::thread([&](){
        response = Call(request, true);
        //Garante o encerramento do socket de feedBack criado para a variável estado caso a comunicação com o AddOn falhe
        if(response==""){
            response="{\"status\":\"fail\", \"packageFile\":\"\", \"package\":\"\"}";
        }
        ciclo=false;
    });
    processo.detach();
    bool estado = iniciarSocketFeedback(ciclo); //recebe e exibe o estado da atividade atual e aguarda o status de erro ou falha antes de avançar
    if(!estado){
        return resultado;
    }
    vector<string> query = stringSplit(&response, '\n');
    string jsonstr = query[0];
    try{
        json j = json::parse(jsonstr);
        string status;
        if(j.contains("status")) j.at("status").get_to(status);
        if(status=="success"){
            string pacoteAddr;
            string pacoete;
            if(j.contains("packageFile")) j.at("packageFile").get_to(pacoteAddr);
            if(j.contains("package")) j.at("package").get_to(pacoete);
            resultado.push_back(pacoteAddr);
            resultado.push_back(pacoete);
        }
    }catch(...){
    }
    return resultado;
}

//Coleção de funções utilitárias para o projeto
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

    //Divide uma string em um vetor de strings com base em um caractere delimitador
    vector<string> stringSplit(std::string* str, char alvo){
        vector<string> arrayfinal;
        std::stringstream ss(*str);
        std::string item;
        while (std::getline(ss, item, alvo)) {
            arrayfinal.push_back(item);
        }
        return arrayfinal;
    }

    //Verifica se uma string contém uma substring
    bool stringContains(std::string* str, std::string alvo){
        return str->find(alvo) != std::string::npos;
    }

    //Verifica se a saída é um terminal para decidir se deve usar cores ou não
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

    //Limpa a saída bruta do comando content query para obter apenas o conteúdo relevante (JSON)
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

    //Verifica se o pacote é compatível com a arquitetura do dispositivo, retorna true se for compatível ou se o pacote for universal (all) e false caso contrário.
    bool checarCompatibilidade(vector<string> abisPacote){
        if(abisPacote[0]=="all") return true;
        string abis = getProp("ro.product.cpu.abilist");
        vector<string> abilista = stringSplit(&abis, ',');
        for(string abichk : abilista)
        for(string abi : abisPacote)
        if(abichk==abi) return true;

        return false;
    }

    //retorna o numero de caracteres que cabe na largura do terminal
    int obterLarguraTerminal() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
    }

    //Gera uma linha inteira de caracteres para preencher a largura do terminal, utilizada para separar seções de informações exibidas no terminal.
    std::string gerarLinhaSeparadora() {
        int largura = obterLarguraTerminal();
        if (largura <= 0) largura = 40; // Fallback caso ioctl falhe
        // Cria uma string com 'largura' vezes o caractere definido
        return std::string(largura, LINHASEPARADORACHAR); 
    }

    //Funções que criam indicadores customizados -----------
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
    //-----------------------------------------------------


    //Se conecta com o LocalServerSocket do java e solicita a resposta para a função
    string requisiçãoViaSocket(string SOCKETNAME, string function, bool aguardarResposta) {
        int sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) return "ERRO_SOCKET";

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;

        // Define o primeiro byte como nulo (Abstract Namespace)
        addr.sun_path[0] = '\0';
        // Copia o nome logo após o nulo
        memcpy(addr.sun_path + 1, SOCKETNAME.c_str(), SOCKETNAME.length());

        // CÁLCULO DO TAMANHO REAL:
        // offsetof(sun_path) nos dá o tamanho da estrutura até chegar na string.
        // + 1 (para o \0 inicial) + o tamanho da string.
        socklen_t len = offsetof(struct sockaddr_un, sun_path) + 1 + SOCKETNAME.length();

        if (connect(sock, (struct sockaddr*)&addr, len) == -1) {
            NLINDERR("INTERNAL_ERROR: ERRO_SOCKET_CONNECT_" + to_string(errno)); // Ex: 111, 13, etc.
            close(sock);
            return "";
        }

        // Enviar comando com \n
        string cmd = function + "\n";
        send(sock, cmd.c_str(), cmd.length(), 0);
        string resposta;
        // Receber resposta se for necessário
        if(aguardarResposta){
            char buffer[4096]; // Buffer de leitura temporário
            //leitura com chunks de buffer para respostas grandes
            int bytes_read;
            while((bytes_read = recv(sock, buffer, sizeof(buffer)-1, 0)) > 0){
                buffer[bytes_read] = '\0';
                resposta.append(buffer);
            }
            if(bytes_read<0){
                NLINDERR("INTERNAL_ERROR: ERRO_SOCKET_RECV_" + to_string(errno)); // Ex: 
            }
        }
        close(sock);
        return resposta;
    }

    //Função para exibir mensagens de erro em formato JSON, utilizada para manter a consistência do formato de saída quando a opção JSON é selecionada.
    void printInfo(string tipo, string menssagem){
        cout << "{\"type\":\""+tipo+"\",\"message\":\""+menssagem+"\"}" << endl; // Exemplo de saída: {"type":"error","message":"Pacote não encontrado"}
    }
}