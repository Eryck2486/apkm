#include "utilitarios.hpp"
#include <sys/system_properties.h>
#include <unistd.h>
#include <sys/stat.h>
#include "idiomas.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <queue>

using json = nlohmann::json;
using namespace std;

RepoConfig::RepoConfig(bool toAdd, std::string url){
    this->toAdd=toAdd;
    this->url=url;
}

RepoConfig::RepoConfig()
{

};

DadosPacote::DadosPacote(string raizrepo, vector<string> dados){
    /*
    std::string pacote;
    std::string nome;
    std::string descrição;
    std::string versão;
    std::string md5sum;
    std::string endereço;
    std::string arquitetura;
    */
    
    this->pacote=dados[0];
    this->nome=dados[1];
    this->descrição=dados[2];
    this->versão=dados[3];
    this->sha256sum=dados[4];
    string enderecoCompleto = dados[5];
    if(!enderecoCompleto.empty()){
        if(enderecoCompleto.find("https://") != std::string::npos){
            this->endereço=enderecoCompleto;
        }else{
            this->endereço=raizrepo+enderecoCompleto;
        }
    }else{
        this->endereço="ERRO_NO_ADDRESS_PROVIDED";
    }
    //Depurando, remover depois
    cout << "Endereço do pacote " << this->pacote << ": " << this->endereço << endl;
    this->arquitetura=dados[6];
}

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
                    r->pacotes.push_back(new DadosPacote(r->repository_sources_path, item.get<std::vector<std::string>>()));
                }
            }
        }
        return r;
    } catch (...) {
        return nullptr;
    }
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

//Equivalente ao getprop nativo do Android
std::string Utilitarios::propertyReader(std::string prop) {
    char value[PROP_VALUE_MAX];
    
    // Tenta obter a localidade atual do sistema
    if (__system_property_get(prop.c_str(), value) > 0) {
        return std::string(value);
    }

    return "none";
}

void Utilitarios::stringReplace(std::string* string, std::string alvo, std::string substituto){
    size_t pos = 0;
    // Loop encontra e substitui até find retornar npos
    while ((pos = string->find(alvo, pos)) != std::string::npos) {
        string->replace(pos, alvo.length(), substituto);
        pos += substituto.length();
    }
}

//Prepara as configurações da execução para o processamento da instrução
Config::Config(int argc, char* argv[]){
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
    stringsidioma = Strings::obterStrings();
    //prepara a instância do CURL para ser utilizada
    curl = curl_easy_init(); //<-- Curl iniciado aqui
    //Passa o comando utilizado para invocar o programa
    nomebinario = argv[0];
    //Percorre e processa a lista de argumentos
    for(int arg=1; arg < argc; arg++){
        string argstr = argv[arg];  
        //1 = atualizar (update)
        //2 = adicionar repositório (--add-repository)
        //3 = remover repositórios inválidos (remove-uknown)
        if(ssl && argstr=="--no-ssl"){
            ssl=false;
        }
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
        delete(config->repoConfig);
        delete(config);
    }
}

//Mostra as configurações da execução
void Config::printcfg(Config* config, Strings* stringsidioma){
    cout << "|-> " << stringsidioma->USANDO_SSL[0] << ": " << boolToHLang(config->ssl, stringsidioma) << endl;
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
Tools::Tools(RepoConfig* repoonfig, Config* configs){
    this->configs=configs;
    this->repoconfig=repoonfig;
}

AddOn::AddOn(string addonpath){
    getConfig();
}

AddOn::~AddOn(){
    delete(config);
}

vector<AddOn*> AddOn::CarregarTodos(Config* config){
    vector<AddOn*> addons = std::vector<AddOn*>();
    try {
        for (const auto& entrada : filesystem::directory_iterator(config->diretórioAddOns)) { // Itera sobre as entradas do diretório
            filesystem::path arquivo = entrada.path();
            if(arquivo.extension()==".so") addons.push_back(new AddOn(arquivo));
        }
    } catch (const filesystem::filesystem_error& e) {
        filesystem::create_directories(config->diretórioAddOns);
        //Lógica para baixar lista de reposiórios da branch master do github
    }
    return addons;
}

void AddOn::getConfig(){
    AddOnConfig* configtmp = new AddOnConfig();
    config = configtmp;
}

RemoteRepoConfig* AddOn::getRepo(){
    return new RemoteRepoConfig();
}

bool Utilitarios::search_match(std::string fonte, std::string termo) {
    // 1. Converte a fonte e o termo para minúsculo para busca insensível
    std::transform(fonte.begin(), fonte.end(), fonte.begin(), ::tolower);
    std::transform(termo.begin(), termo.end(), termo.begin(), ::tolower);

    // 2. Verifica se o termo existe dentro da fonte
    if (fonte.find(termo) != std::string::npos) {
        return true;
    }
    
    return false;
}

std::string Utilitarios::obterPastaTemporaria() {
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

    string apkmtmp = "/tmp/apkm-tmp";
    if(!filesystem::exists(apkmtmp)){
        filesystem::create_directories(apkmtmp);
    }
    return apkmtmp;
}