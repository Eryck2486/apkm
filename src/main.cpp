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
#include "gerenciador-pacotes.hpp"

using namespace std;

int Main::main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_ALL);
    Config* config = new Config(argc, argv);
    Repomanager* manager = new Repomanager(config);
    GerenciadorPacotes* gerenciador = new GerenciadorPacotes(config, manager);
    int retorno = 1;
    Config::printcfg(config, config->stringsidioma);
    if (config->curl){
        switch(config->instrução){
            case 1:
                break;
            case 2:
                if(manager->adicionarRepositório()){
                    retorno = 0;
                }
                break;
            case 3:
                if(manager->removerRepositório()){
                    retorno = 0;
                }
                break;
            case 4:
                if(manager->listarRepositórios()){
                    retorno = 0;
                }
                break;
            case 5:
                //listar AddOns
                retorno = 0;
                break;
            case 6:
                if(gerenciador->pesquisar()){
                    retorno = 0;
                }
                break;
            case 7:
                if(gerenciador->instalarPacotes()){
                    retorno = 0;
                }
                break;
            default:
                printhelp(config);
                retorno=1;
                break;
        }
    }
    delete(manager);
    delete(config);
    curl_global_cleanup();
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