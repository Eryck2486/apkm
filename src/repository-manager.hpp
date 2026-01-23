
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
class repomanager{
    public:
        repomanager(Config* config){
           this->configs = config;
        }
        Config* configs;
        std::vector<RepoConfig*> ObterRepositórios(Config* config);
        bool validar(Tools& tool);
        bool adicionarRepositório(Config* conf);
        bool atualizarRepositórios(Config* conf);
        bool removerInvalidos(std::vector<RepoConfig*> repos);
    private:
        static void save_or_update(RepoConfig* myConfig);
        static std::string sources_dir(std::string sourcename);
        static std::string digestToHex(unsigned char* hash, unsigned int len);
        static CURLcode sslctx_function_adapter(CURL* curl, void* sslctx, void* parm);
        static size_t write_null_callback(void *contents, size_t size, size_t nmemb, void *userp);
        static int verify_callback(X509_STORE_CTX* ctx, void* arg);
        static bool compareHashes(std::vector<std::string> dominioHashs, std::vector<std::string> hashesLocais);
        static std::vector<std::string> GetHashs(Tools* tool, X509_STORE_CTX* ctx);
        static int verificar_dominio(X509* cert, const std::string& hostname);
        static bool is_valid_now(X509* cert, RepoConfig& config);
        static std::string gethostname(std::string url);
};

