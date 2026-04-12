#include <iostream> // Para std::cout
#include <curl/curl.h>
#include "apkm.hpp"
#include "repository_manager.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/sha.h>
#include <iomanip>
#include "main.hpp"
#include "idiomas.hpp"
#include "gerenciador_pacotes.hpp"

using namespace std;
using namespace Utilitarios;

int Main::main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_ALL);
    Config* config = new Config(argc, argv);
    Repomanager* manager = new Repomanager(config);
    GerenciadorPacotes* gerenciador = new GerenciadorPacotes(config, manager);
    int retorno = 1;
    Config::printcfg(config, config->stringsidioma);
    if (config->curl){
        //Verifica se existe alguma fonte antes de prosseguir
        if(config->reposglobais.size()>0 || config->addonsdinamicos.size()>0){
            switch(config->instrução){
                case 1:
                    gerenciador->updatePacotes();
                    retorno = 0;
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
                    showAddOnsInfos(config);
                    retorno = 0;
                    break;
                case 6:
                    gerenciador->pesquisar();
                    retorno = 0;
                    break;
                case 7:
                    if(gerenciador->prepararInstalarPacotes(config->nomes)){
                        retorno = 0;
                    }
                    break;
                case 8:
                    if(gerenciador->desinstalarPacotes()){
                        retorno = 0;
                    }
                    break;
                default:
                    printhelp(config);
                    retorno=1;
                    break;
            }
        }else{
            NLINDERR(config->stringsidioma->ERR_NO_REPOS[0]);
            retorno=1;
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
    stringReplace(&ajudastr, "$$BIN", conf->nomebinario);
    cout << ajudastr;
}

void Main::showAddOnsInfos(Config* conf){
    vector<AddOn*> addons = AddOn::CarregarTodos(conf);
    for(AddOn* addon : addons){
        addon->exibirAddOnInfos(conf);
        gerarLinhaSeparadora();
    }
}

int main(int argc, char* argv[])
{
    Main* main = new Main();
    return main->main(argc, argv);
}