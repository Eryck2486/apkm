#include <curl/curl.h>
#include <openssl/ssl3.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include "repository-manager.hpp"
#include <filesystem>
#include <queue>
#include <chrono>
#include <fstream>
#include <vector>

using namespace std;
using json = nlohmann::json;

//Construtor para struct tools
Tools::Tools(RepoConfig repoonfig, Config* configs){
    this->configs=configs;
    this->repoconfig=repoonfig;
}

//Obtem a lista de repositórios a partir da pasta de repositórios
std::vector<RepoConfig*> repomanager::ObterRepositóriosValidos(Config* configs){
    Strings* s = configs->stringsidioma;
    cout << s->CARREGANDO_REPOSITORIOS[0] << endl;
    std::queue<std::string> repoFiles;
    try {
        for (const auto& entrada : filesystem::directory_iterator(sources_dir(""))) { // Itera sobre as entradas do diretório
            filesystem::path arquivo = entrada.path();
            if(arquivo.extension()==".json") repoFiles.push(arquivo);
        }
    } catch (const filesystem::filesystem_error& e) {
        std::cerr << s->ERRO_REPOS_N_ENCONTRADOS[0];
        filesystem::create_directories(sources_dir(""));
        //Lógica para baixar lista de reposiórios da branch master do github
    }
    
    vector<RepoConfig*> repositorios;
    //converte
    while(repoFiles.size() > 0){
        string file = repoFiles.front();
        repoFiles.pop();
        ifstream arquivostream(file);
        string jsonstr;
        string tmp;
        while (getline(arquivostream, tmp)){
            jsonstr.append(tmp+"\n");
        }
        json j = json::parse(jsonstr);
        RepoConfig* repoconfig = new RepoConfig();
        from_json(j, *repoconfig);
        cout << s->VERIFICANDO_REPOSITORIO[0] << repoconfig->name << " (" << repoconfig->url << "):" << endl;
        Tools* tool = new Tools(
            *repoconfig,
            configs
        );
        if(validar(*tool)){
            repositorios.push_back(repoconfig);
        }
        delete(repoconfig);
    }
    return repositorios;
}

//Verifica a validade de um domínio HTTPS
bool repomanager::validar(Tools& tools) {
    CURL* curl = tools.configs->curl;
    if (!curl) return false;
    // Reset para limpar configurações de chamadas anteriores
    curl_easy_reset(curl);

    // REDIRECIONA A SAÍDA: Isso impede que o HTML apareça no terminal
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_null_callback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tools.serverResponse);

    curl_easy_setopt(curl, CURLOPT_URL, tools.repoconfig.url.c_str());
    
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslctx_function_adapter);

    // Passa o endereço da struct Tool para o parm do callback
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, &tools);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);

    CURLcode res = curl_easy_perform(curl);
    
    if(!tools.repoconfig.valido){
        cout << tools.repoconfig.errMsg << endl;
    }
    return tools.repoconfig.valido;
}

//Adiciona o repositório
bool repomanager::adicionarRepositório(Config* conf){
    RepoConfig* repoconfig = new RepoConfig(
        true,
        *conf->url
    );
}

//Atualiza os repositórios
bool repomanager::atualizarRepositórios(Config* conf){
    vector<RepoConfig*> repos = ObterRepositóriosValidos(conf);
    
}

//Salva ou atualiza um repositório caso o arquivo já exista
void repomanager::save_or_update(const RepoConfig& myConfig) {
    // 1. Converte a struct em um objeto JSON
    json j;
    to_json(j, myConfig);

    // 2. Transforma o objeto JSON em string
    // O argumento '4' dentro de dump() serve para identação (pretty print)
    // Se quiser uma string compacta (para economizar espaço no Android), use dump() sem argumentos.
    std::string jsonString = j.dump(4);

    // Agora você pode usar sua função de gravar no arquivo
    // gravar_arquivo("repo.json", jsonString);
    std::string arquivorepo = sources_dir(myConfig.name);
    ofstream repostream(arquivorepo);
    if(repostream.is_open()){
        repostream << jsonString;
    }
}

//Converte string JSON para estrutura local RepoConfig
void repomanager::from_json(const json& j, RepoConfig& r) {
    // Mapeamento simples
    j.at("repo_name").get_to(r.name);
    j.at("url").get_to(r.url);

    // Mapeamento de campo aninhado (security -> allowed_hashes)
    if (j.contains("security") && j["security"].contains("allowed_hashes")) {
        j.at("security").at("allowed_hashes").get_to(r.pinned_hashes);
    }
}

//Converte os dados da estrutura RepoConfig para string JSON
void repomanager::to_json(json& j, const RepoConfig& r) {
    j = json{
        {"repo_name", r.name},
        {"url", r.url},
        {"security", {
            {"allowed_hashes", r.pinned_hashes},
            {"check_expiry", true} // Valor padrão
        }}
    };
}

//Retorna o endereço dos repositórios
string repomanager::sources_dir(std::string sourcename){
    if(sourcename!=""){
        return "/data/apkm-sources/"+sourcename;
    }
    return "/data/apkm-sources";
} 

// Converte o digest binário para string Hexadecimal
std::string repomanager::digestToHex(unsigned char* hash, unsigned int len) {
    std::stringstream ss;
    for (unsigned int i = 0; i < len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

//Função auxiliar para verify_callback
CURLcode repomanager::sslctx_function_adapter(CURL* curl, void* sslctx, void* parm) {
    SSL_CTX* ctx = (SSL_CTX*)sslctx;
    
    // Esta função define o callback e passa o parm (RepoConfig) como o 'arg' do verify_callback
    SSL_CTX_set_cert_verify_callback(ctx, verify_callback, parm);
    
    return CURLE_OK;
}

// Função para descartar ou capturar a saída do servidor
size_t repomanager::write_null_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Se você quiser capturar o JSON do repo futuramente:
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int repomanager::verify_callback(X509_STORE_CTX* ctx, void* arg) {
    Tools* tool = (Tools*)arg;
    Strings* strings = tool->configs->stringsidioma;
    RepoConfig* config = &tool->repoconfig;

    //Obtém a cadeia de certificados
    STACK_OF(X509)* chain = X509_STORE_CTX_get0_chain(ctx);
    X509* cert = nullptr;

    if (chain && sk_X509_num(chain) > 0) {
        cert = sk_X509_value(chain, 0); //Certificado folha (do site)
    } else {
        cert = X509_STORE_CTX_get0_cert(ctx);
    }

    if (!cert) {
        config->errMsg = "|->[SSL_ERR_46]: "+strings->ERRO_OBTER_CERTIFICADO[0]+"\n";
        return 0;
    }

    if(config->check_expiry){
        if(is_valid_now(cert, *config)){
            config->valido=true;
        };
    }
    int estadodominio = verificar_dominio(cert, gethostname(config->url));
    if(estadodominio==0){
        config->errMsg = "|-> [SSL_ERR_42]: "+strings->CERT_NAO_CONFIAVEL[0]+".\n";
        return 0;
    }else if(estadodominio<0){
        config->errMsg = "|-> [SSL_ERR_46]: "+strings->ERRO_OBTER_CERTIFICADO[0]+".\n";
        return 0;
    }
    //Calcula o Digest (Hash)
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len = 0;
    if (X509_digest(cert, EVP_sha256(), hash, &hash_len)) {
        //Converte para Hexadecimal localmente para evitar erros de escopo
        std::stringstream ss;
        for (unsigned int i = 0; i < hash_len; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        std::string current_hash = ss.str();
        if(config->toAdd){
            config->pinned_hashes.push_back(current_hash);
        }
        if (!config) {
            return 0;
        }

        //Comparação
        for (const auto& pinned : config->pinned_hashes) {
            if (current_hash == pinned) {
                config->valido=true;
                cout << "|-> " << strings->PRONTO[0] << endl;
                return 1;
            }
        }
    }
    config->errMsg = "|-> [SSL_ERR_46]: "+strings->ERRO_HASHS_DESCONHECIDOS[0]+"\""+tool->configs->nomebinario+" remove uknown\""+strings->ERRO_HASHS_DESCONHECIDOS[1]+"\""+tool->configs->nomebinario+" update "+config->name+" --no-ssl\""+strings->ERRO_HASHS_DESCONHECIDOS[2]+"\n";
    return 0;
}

int repomanager::verificar_dominio(X509* cert, const std::string& hostname) {
    // X509_check_host: Verifica o host contra SAN/CN.
    // O último parâmetro (char **peername) pode ser usado para obter o nome do peer (NULL aqui).
    int result = X509_check_host(cert, hostname.c_str(), hostname.length(), 0, NULL);
    
    return result;
}

//Verifica se a data de validade do certificado é válida se comparada com a data do sistema
bool repomanager::is_valid_now(X509* cert, RepoConfig& config) {
    // > 0 se o tempo do cert é POSTERIOR ao tempo fornecido
    if (X509_cmp_time(X509_get0_notBefore(cert), NULL) > 0) {
        // O certificado só será válido no futuro
        config.errMsg="|->[SSL_ERR_43: A data de validade do certificado ainda não é válida.\n]";
        return false;
    }

    //Verificar se o certificado já expirou (Not After)
    if (X509_cmp_time(X509_get0_notAfter(cert), NULL) < 0) {
        // O tempo atual já passou da data de expiração
        config.errMsg="|->[SSL_ERR_45: A data de validade do certificado expirou.\n]";
        return false;
    }

    return true; // Está dentro do prazo de validade
}

//obtem o domínio de uma URL
std::string repomanager::gethostname(string url) {
    CURLU *url_handle;
    CURLUcode rc;
    char *host = nullptr;

    // 1. Inicializar o handle
    url_handle = curl_url();

    if (url_handle) {
        // 2. Definir a URL (o parse acontece aqui)
        rc = curl_url_set(url_handle, CURLUPART_URL, url.c_str(), 0);

        if (rc == CURLUE_OK) {
            // 3. Obter apenas o host (domínio)
            rc = curl_url_get(url_handle, CURLUPART_HOST, &host, 0);

            if (rc == CURLUE_OK) {                
                // Liberar a string alocada pela libcurl
                curl_free(host);
            }
        }
        
        // 4. Limpar o handle de URL
        curl_url_cleanup(url_handle);
    }

    return host;
}