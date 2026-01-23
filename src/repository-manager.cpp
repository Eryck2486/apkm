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
Tools::Tools(RepoConfig* repoonfig, Config* configs){
    this->configs=configs;
    this->repoconfig=repoonfig;
}

//Obtem a lista de repositórios a partir da pasta de repositórios
std::vector<RepoConfig*> repomanager::ObterRepositórios(Config* configs){
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
        RepoConfig* repoconfig = RepoConfig::from_json(jsonstr);
        repoconfig->filepath=file;
        cout << s->VERIFICANDO_REPOSITORIO[0] << repoconfig->name << " (" << repoconfig->url << "):" << endl;
        repositorios.push_back(repoconfig);
    }
    return repositorios;
}

//Verifica a validade de um domínio HTTPS
bool repomanager::validar(Tools& tools) {
    CURL* curl = tools.configs->curl;
    if (!curl) return false;

    // REDIRECIONA A SAÍDA: Isso impede que o HTML apareça no terminal
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_null_callback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tools.serverResponse);

    curl_easy_setopt(curl, CURLOPT_URL, (tools.repoconfig->url+"/index.json").c_str());
    
    // Passa o endereço da struct Tool para o parm do callback
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, &tools);

    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslctx_function_adapter);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);

    curl_easy_perform(curl);
    
    if(tools.repoconfig->valido){
        return true;
    }
    cout << tools.repoconfig->errMsg << endl;
    return false;
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
    vector<RepoConfig*> repos = ObterRepositórios(conf);
    for(RepoConfig* repo : repos){
        Tools tool = Tools(repo, conf);
        if(validar(tool)){
            save_or_update(repo);
        }
        RemoteRepoConfig* onlineconfig = RemoteRepoConfig::fromJson(tool.serverResponse);
        if(onlineconfig){
            cout << "Lendo repositório" << endl <<
            "Nome: " << onlineconfig->name << endl <<
            "URL: " << onlineconfig->repository_sources_path << endl;
            for(vector<string> packagedata : onlineconfig->packages){
                cout << "Pacote: pack: " << packagedata[0] << ", Name: " << packagedata[1] << ", Desc: " << packagedata[2] << ",End: " << packagedata[3] << ", Arch: " << packagedata[4] << endl;
            }
        }else{

        }
    }
    return true;
}

bool repomanager::removerInvalidos(std::vector<RepoConfig*> repos){
    for(RepoConfig* repo : repos){
        if(!repo->valido){
            filesystem::remove(repo->filepath);
        }
    }
}

//Salva ou atualiza um repositório caso o arquivo já exista
void repomanager::save_or_update(RepoConfig* myConfig) {
    std::string jsonString=myConfig->to_json();
    cout << jsonString << endl;
    std::string arquivorepo = myConfig->filepath;
    ofstream repostream(arquivorepo);
    if(repostream.is_open()){
        cout << "Gravando \"" << arquivorepo << "\"" << endl;
        repostream << jsonString << endl;
        repostream.close();
        cout << "\"" << arquivorepo << "\" gravado" << endl;
    }else{
        cout << "Erro ao abrir \"" << arquivorepo << "\"" << endl;
    }
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
    vector<string> dominioHashs = GetHashs(tool, ctx);
    switch(tool->configs->instrução){
        case 1:
            {
                if(tool->configs->ssl && compareHashes(dominioHashs, tool->repoconfig->pinned_hashes)){
                    tool->repoconfig->pinned_hashes.clear();
                    tool->repoconfig->pinned_hashes=dominioHashs;
                    tool->repoconfig->valido=true;
                    return 1;
                }else if(!tool->configs->ssl){
                    tool->repoconfig->pinned_hashes.clear();
                    tool->repoconfig->pinned_hashes=dominioHashs;
                    tool->repoconfig->valido=true;
                    return 1;
                }
            }
            break;
    }
    tool->repoconfig->errMsg = "[SSL] Nenhum hash da cadeia coincide com os pins configurados.\n";
    tool->repoconfig->valido = false;
    return 0; // Bloqueia a conexão
}

bool repomanager::compareHashes(std::vector<std::string> dominioHashs, std::vector<std::string> hashesLocais){
    for(string hashLocal : hashesLocais){
        for(string hashDominio : dominioHashs){
            if(hashLocal == hashDominio) return true;
        }
    }
    return false;
}

//Obtem os hashes dos certificados do servidor
vector<string> repomanager::GetHashs(Tools* tool, X509_STORE_CTX* ctx){
    vector<string> hashes;
    bool dominioCertificado = false;
    //Obtemos o certificado folha (o principal do site)
    X509* leaf_cert = X509_STORE_CTX_get0_cert(ctx);
    
    //Obtemos a pilha de certificados extras que o servidor enviou (intermediários)
    STACK_OF(X509)* untrusted_stack = X509_STORE_CTX_get0_untrusted(ctx);

    //Criamos um vetor temporário para facilitar a iteração de todos eles
    std::vector<X509*> all_certs;
    if (leaf_cert) all_certs.push_back(leaf_cert);

    if (untrusted_stack) {
        for (int i = 0; i < sk_X509_num(untrusted_stack); i++) {
            all_certs.push_back(sk_X509_value(untrusted_stack, i));
        }
    }

    // 3. Agora iteramos por TODOS os certificados encontrados na conexão
    for (X509* cert : all_certs) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        unsigned int hash_len = 0;
        if (X509_digest(cert, EVP_sha256(), hash, &hash_len)) {
            if(verificar_dominio(cert, gethostname(tool->repoconfig->url))) dominioCertificado=true;
            std::string current_hash = digestToHex(hash, hash_len);
            hashes.push_back(current_hash);
        }
    }
    if(!dominioCertificado){
        hashes.clear();
    }
    return hashes;
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

    char *hosttmp = nullptr;
    std::string resultado;

    // 1. Inicializar o handle
    url_handle = curl_url();

    if (url_handle) {
        // 2. Definir a URL (o parse acontece aqui)
        rc = curl_url_set(url_handle, CURLUPART_URL, url.c_str(), 0);

        if (rc == CURLUE_OK) {
            // 3. Obter apenas o host (domínio)
            rc = curl_url_get(url_handle, CURLUPART_HOST, &hosttmp, 0);

            if (rc == CURLUE_OK) {                
                // Liberar a string alocada pela libcurl
                resultado=hosttmp;
                curl_free(hosttmp);
            }
        }
        
        // 4. Limpar o handle de URL
        curl_url_cleanup(url_handle);
    }

    return resultado;
}