#include "utilitarios.hpp"
#include <sys/system_properties.h>
#include "idiomas.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

RepoConfig::RepoConfig(bool toAdd, std::string url){
    this->toAdd=toAdd;
    this->url=url;
}

RepoConfig::RepoConfig()
{

};

RemoteRepoConfig* RemoteRepoConfig::fromJson(string jsonstring){
    RemoteRepoConfig* config = new RemoteRepoConfig();
    json j = json::parse(jsonstring);
    if(j.contains("name") && j.contains("repository_sources_path") && j.contains("packages")){
        j.at("name").get_to(config->name);
        j.at("repository_sources_path").get_to(config->repository_sources_path);
        if (j["packages"].is_array()) {
            j.at("packages").get_to(config->packages);
        }
    }
    return config;
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
    //Prepara o suporte de idioma do sistema
    stringsidioma = Strings::obterStrings();
    //prepara a instância do CURL para ser utilizada
    curl = curl_easy_init();
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

        if(argstr=="update"){
            instrução=1;
        }else if(argstr=="--add-repository"){
            instrução=2;
        }else if(argstr=="remove-uknown"){
            instrução=3;
        }else{
            comandoInvalido=argstr;
        }
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