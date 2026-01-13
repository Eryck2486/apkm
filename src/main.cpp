#include <iostream> // Para std::cout
#include <curl/curl.h>
#include "utilitarios.hpp"
#include "repository-manager.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/sha.h>
#include <iomanip>
#include "main.hpp"
#include "idiomas.hpp"

using namespace std;

int Main::main(int argc, char* argv[]) {
    Config* config = new Config(argc, argv);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    repomanager* manager = new repomanager(config);
    if (config->curl){
        vector<RepoConfig*> repos = manager->ObterRepositóriosValidos(config);
        
        return 0;
    }
    return 1;
}

int main(int argc, char* argv[])
{
    Main* main = new Main();
    return main->main(argc, argv);
}