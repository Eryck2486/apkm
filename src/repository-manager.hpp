
#pragma once
#include <curl/curl.h>
#include <iostream>
#include <vector>
#include <openssl/types.h>
#include <nlohmann/json.hpp>
#include "utilitarios.hpp"

using json = nlohmann::json;
// Forward declarations
struct RepoConfig;
struct Config;



//Classe principal do gerenciador de repositórios
class Repomanager{
public:
    Repomanager(Config* config);
    Config* configs;
    std::vector<RemoteRepoConfig*> ObterTodosRepositórios();
    std::vector<RepoConfig*> CarregarRepositóriosLocais(Config* config);
    bool validar(Tools& tool);
    bool adicionarRepositório();
    bool removerRepositório();
    bool listarRepositórios();
    bool baixar(const std::string& url, const std::string& destino, bool mostrarProgresso);
private:
    static void save_or_update(RepoConfig* myConfig);
    std::string sources_dir(std::string sourcename);
    static std::string digestToHex(unsigned char* hash, unsigned int len);
    static CURLcode sslctx_function_adapter(CURL* curl, void* sslctx, void* parm);
    static size_t write_null_callback(void *contents, size_t size, size_t nmemb, void *userp);
    static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream);
    static int verify_callback(X509_STORE_CTX* ctx, void* arg);
    static bool compareHashes(std::vector<std::string> dominioHashs, std::vector<std::string> hashesLocais);
    static std::vector<std::string> GetHashs(Tools* tool, X509_STORE_CTX* ctx);
    static int verificar_dominio(X509* cert, const std::string& hostname);
    static bool is_valid_now(X509* cert, RepoConfig& config);
    static std::string gethostname(std::string url);
    static int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
};

