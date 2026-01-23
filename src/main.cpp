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
    int retorno = 1;
    if (config->curl){
        switch(config->instrução){
            case 1: 
                {   
                    Config::printcfg(config, config->stringsidioma);
                    bool conclusão = manager->atualizarRepositórios(config);
                }
                retorno = 0;
                break;
            case 2:
                {
                    
                }
                retorno = 0;
                break;
            case 3:
                {
                    Config::printcfg(config, config->stringsidioma);
                    vector<RepoConfig*> repos = manager->ObterRepositórios(config);
                    manager->removerInvalidos(repos);
                }
                retorno = 0;
                break;
            default:
                printhelp(config);
                break;
        }
    }
    delete(manager, config);
    return retorno;
}

void Main::printhelp(Config* conf){
    Strings* idioma = conf->stringsidioma;
    if(conf->comandoInvalido!=""){
        cout << idioma->ERRO_COM_INVALIDO[0] << conf->comandoInvalido << idioma->ERRO_COM_INVALIDO[1];
    }
    string ajudastr = idioma->AJUDA[0];
    Utilitarios::stringReplace(&ajudastr, "$$BIN", conf->nomebinario);
    cout << ajudastr;
}

int main(int argc, char* argv[])
{
    Main* main = new Main();
    return main->main(argc, argv);
}