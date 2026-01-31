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

Repomanager::Repomanager(Config* config)
{
    this->configs = config;
    this->configs->reposglobais=ObterTodosRepositórios();
}

//Carrega todos os repositórios disponíveis em um único Array
vector<RemoteRepoConfig*> Repomanager::ObterTodosRepositórios(){
    //lista que será retornada ao final
    vector<RemoteRepoConfig*> reposglobais;
    //Carregando repositórios do "disco"
    vector<RepoConfig*> reposlocais = CarregarRepositóriosLocais(configs);
    for(RepoConfig* repo : reposlocais){
        Tools tool = Tools(repo, configs);
        if(validar(tool)){
            RemoteRepoConfig* serverRepo = RemoteRepoConfig::fromJson(tool.serverResponse);
            filesystem::path origem = repo->filepath;
            serverRepo->origem=origem.filename();
            if(serverRepo){
                reposglobais.push_back(serverRepo);
            }
        }
        delete(repo);
    }

    //Lógica para carregar RemoteRepoConfig de AddOns
    for(AddOn* addon : AddOn::CarregarTodos(configs)){
        if(addon->config->dinamico){
            configs->addonsdinamicos.push_back(addon);
        }else{
            reposglobais.push_back(addon->getRepo());
        }
    }

    if(configs->instrução==1){

    }
    //
    return reposglobais;
}

//Obtem a lista de repositórios a partir da pasta de repositórios locais
std::vector<RepoConfig*> Repomanager::CarregarRepositóriosLocais(Config* configs){
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
        repositorios.push_back(repoconfig);
    }
    return repositorios;
}

//Verifica a validade de um domínio HTTPS
bool Repomanager::validar(Tools& tools) {
    CURL* curl = tools.configs->curl;
    if (!curl) return false;

    // 1. LIMPA o estado anterior do handle para esta nova requisição
    curl_easy_reset(curl);

    // 2. Garante que a string de resposta esteja vazia
    tools.serverResponse = "";
    tools.repoconfig->valido = false;

    long http_code = 0;

    // 3. Configura TUDO novamente (obrigatório após o reset)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_null_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tools.serverResponse);
    
    std::string full_url = tools.repoconfig->url + "/index.json";
    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
    
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, &tools);
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslctx_function_adapter);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "APKM-Manager/1.0");

    curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L); // Desativa o cache de sessão SSL
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);       // Força uma nova conexão TCP
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);        // Fecha a conexão após o uso

    // 4. Executa
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        tools.repoconfig->errMsg = curl_easy_strerror(res);
        return false;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (tools.repoconfig->valido && http_code >= 200 && http_code < 300) {
        return true;
    }

    // Se chegou aqui, deu erro
    if (http_code != 200) {
        cout << http_code << "." << endl;
        tools.repoconfig->errMsg = tools.configs->stringsidioma->REPOSITORIO_INVALIDO[0];
    }
    return false;
}

//Adiciona o repositório
bool Repomanager::adicionarRepositório(){
    RepoConfig* repoconfig = new RepoConfig(
        true,
        configs->url
    );
    Tools tool = Tools(repoconfig, configs);
    cout << configs->stringsidioma->VERIFICANDO_REPOSITORIO[0]+configs->url+": ";
    if(validar(tool)){
        cout << configs->stringsidioma->PRONTO[0] << endl;
        RemoteRepoConfig* onlineconfig = RemoteRepoConfig::fromJson(tool.serverResponse);
        if(onlineconfig){
            repoconfig->name=onlineconfig->name;
            repoconfig->filepath=sources_dir(repoconfig->name+".json");
            cout << configs->stringsidioma->ADICIONANDO_REPOSITÒRIOS[0] << repoconfig->name << configs->stringsidioma->ADICIONANDO_REPOSITÒRIOS[1] << endl;
            if (cin.peek() == '\n') {
                cin.ignore();
            }
            save_or_update(repoconfig);
            cout << configs->stringsidioma->REPOSITORIO_ADICIONADO[0] << endl;
            return true;
        }else{
            cout << configs->stringsidioma->REPOSITORIO_INVALIDO[0];
            return false;
        }
    }else{
        cerr << repoconfig->errMsg << endl;
    }
    return false;
}

bool Repomanager::removerRepositório(){
    vector<RepoConfig*> repos = CarregarRepositóriosLocais(configs);
    for(string nome : configs->nomes){
        bool encontrado = false;
        for(RepoConfig* repo : repos){
            if(repo->name==nome){
                if(filesystem::exists(repo->filepath))
                {
                    filesystem::remove(repo->filepath);
                    cout << configs->stringsidioma->REPOSITORIO_REMOVIDO[0] << repo->name << configs->stringsidioma->REPOSITORIO_REMOVIDO[1] << endl;
                    encontrado=true;
                    break;
                }

            }
        }
        if(!encontrado){
            cerr << configs->stringsidioma->REPOSITORIO_N_ENCONTRADO[0] << nome << configs->stringsidioma->REPOSITORIO_N_ENCONTRADO[1] << endl;
            return false;
        }
    }
    return true;
}

bool Repomanager::listarRepositórios(){
    vector<RemoteRepoConfig*> repos = configs->reposglobais;
    if(repos.size()>0){
        for(RemoteRepoConfig* repo : configs->reposglobais){
            cout << repo->name << " (" << repo->origem << "): " << endl;
        }
    }else{
        cerr << configs->stringsidioma->NENHUM_REPO_ENCONTRADO[0] << endl;
        return false;
    }
    return true;
}

bool Repomanager::baixar(const std::string& url, const std::string& destino, bool mostrarProgresso) {
    CURL* curl = configs->curl;
    curl_easy_reset(curl);

    FILE* fp = fopen(destino.c_str(), "wb");
    if (!fp) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    // Timeout: não fica esperando para sempre (ex: 30s para conectar)
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);

    if (mostrarProgresso) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);

    if (res != CURLE_OK) {
        // Se falhou, remove o arquivo incompleto/corrompido
        std::filesystem::remove(destino);
        return false;
    }
    return true;
}

//Salva ou atualiza um repositório caso o arquivo já exista
void Repomanager::save_or_update(RepoConfig* myConfig) {
    std::string jsonString=myConfig->to_json();
    std::string arquivorepo = myConfig->filepath;
    ofstream repostream(arquivorepo);
    if(repostream.is_open()){
        repostream << jsonString << endl;
        repostream.close();
    }else{
        
    }
}

//Retorna o endereço dos repositórios
string Repomanager::sources_dir(std::string sourcename){
    if(sourcename!=""){
        return (configs->diretórioSources+"/"+sourcename);
    }
    return configs->diretórioSources;
}



// Converte o digest binário para string Hexadecimal
std::string Repomanager::digestToHex(unsigned char* hash, unsigned int len) {
    std::stringstream ss;
    for (unsigned int i = 0; i < len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

//Função auxiliar para verify_callback
CURLcode Repomanager::sslctx_function_adapter(CURL* curl, void* sslctx, void* parm) {
    SSL_CTX* ctx = (SSL_CTX*)sslctx;
    
    // Esta função define o callback e passa o parm (RepoConfig) como o 'arg' do verify_callback
    SSL_CTX_set_cert_verify_callback(ctx, verify_callback, parm);
    
    return CURLE_OK;
}

// Função para descartar ou capturar a saída do servidor
size_t Repomanager::write_null_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Se você quiser capturar o JSON do repo futuramente:
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t Repomanager::write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int Repomanager::verify_callback(X509_STORE_CTX* ctx, void* arg) {
    Tools* tool = (Tools*)arg;
    vector<string> dominioHashs = GetHashs(tool, ctx);
    switch(tool->configs->instrução)
    {
        case 1:
            {
                if(tool->configs->ssl && compareHashes(dominioHashs, tool->repoconfig->pinned_hashes)){
                    tool->repoconfig->pinned_hashes.clear();
                    tool->repoconfig->pinned_hashes=dominioHashs;
                    tool->repoconfig->valido=true;
                    cout << "At" << endl;
                    return 1;
                }else if(!tool->configs->ssl){
                    tool->repoconfig->pinned_hashes.clear();
                    tool->repoconfig->pinned_hashes=dominioHashs;
                    tool->repoconfig->valido=true;
                    cout << "At" << endl;
                    return 1;
                }
                tool->repoconfig->errMsg = tool->configs->stringsidioma->ERRO_HASHS_DESCONHECIDOS[0]+"\""+tool->configs->nomebinario+" --remove "+tool->repoconfig->name+"\""+tool->configs->stringsidioma->ERRO_HASHS_DESCONHECIDOS[1]+"\""+tool->configs->nomebinario+" --update "+tool->repoconfig->name+" --no-ssl"+"\""+tool->configs->stringsidioma->ERRO_HASHS_DESCONHECIDOS[2];
            }
        case 2:
            {
                tool->repoconfig->pinned_hashes.clear();
                tool->repoconfig->pinned_hashes=dominioHashs;
                tool->repoconfig->valido=true;
                return 1;
            }
        default:
            if(compareHashes(tool->repoconfig->pinned_hashes, dominioHashs)){
                tool->repoconfig->valido=true;
                return 1;
            }    
    }
    tool->repoconfig->valido = false;
    return 0; // Bloqueia a conexão
}

bool Repomanager::compareHashes(std::vector<std::string> dominioHashs, std::vector<std::string> hashesLocais){
    for(string hashLocal : hashesLocais){
        for(string hashDominio : dominioHashs){
            if(hashLocal == hashDominio) return true;
        }
    }
    return false;
}

//Obtem os hashes dos certificados do servidor
vector<string> Repomanager::GetHashs(Tools* tool, X509_STORE_CTX* ctx){
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
            if(verificar_dominio(cert, gethostname(tool->repoconfig->url)) && is_valid_now(cert, *tool->repoconfig)) dominioCertificado=true;
            std::string current_hash = digestToHex(hash, hash_len);
            hashes.push_back(current_hash);
        }
    }
    if(!dominioCertificado){
        hashes.clear();
    }
    return hashes;
}

int Repomanager::verificar_dominio(X509* cert, const std::string& hostname) {
    // X509_check_host: Verifica o host contra SAN/CN.
    // O último parâmetro (char **peername) pode ser usado para obter o nome do peer (NULL aqui).
    int result = X509_check_host(cert, hostname.c_str(), hostname.length(), 0, NULL);
    
    return result;
}

//Verifica se a data de validade do certificado é válida se comparada com a data do sistema
bool Repomanager::is_valid_now(X509* cert, RepoConfig& config) {
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
std::string Repomanager::gethostname(string url) {
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

int Repomanager::progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if (dltotal <= 0) return 0; // Evita divisão por zero no início

    // Calcula a porcentagem
    double percentage = (double)dlnow / (double)dltotal * 100.0;
    int width = 30; // Largura da barra em caracteres
    int pos = width * (percentage / 100.0);

    // Desenha a barra no terminal
    std::cout << "\r["; // \r volta o cursor para o início da linha
    for (int i = 0; i < width; ++i) {
        if (i < pos) std::cout << "#";
        else if (i == pos) std::cout << ">";
        else std::cout << "-";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << percentage << "%" << std::flush;

    return 0; // Retorne 0 para continuar o download, ou 1 para abortar
}