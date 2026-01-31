#include "gerenciador-pacotes.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "utilitarios.hpp"
using namespace std;
bool GerenciadorPacotes::pesquisar(){
    vector<DadosPacote*> resultados;
    string pesqcmresult;
    for(string pesquisa : configs->nomes){
        bool encontrado = false;
        for(RemoteRepoConfig* repoconfig : configs->reposglobais){
            for(DadosPacote* pacote : repoconfig->pacotes){
                if(
                    Utilitarios::search_match(pacote->descrição, pesquisa) ||
                    Utilitarios::search_match(pacote->nome, pesquisa) ||
                    Utilitarios::search_match(pacote->pacote, pesquisa)
                ){
                    resultados.push_back(pacote);
                    if(pesqcmresult=="" && !encontrado){
                        pesqcmresult=pesquisa;
                    }else if(!encontrado){
                        pesqcmresult.append(", "+pesquisa);
                    }
                    encontrado=true;
                }
            }
        }
    }

    if(resultados.size()>0){
        cout << configs->stringsidioma->MOSTR_RESULTS[0] << pesqcmresult << endl;
        for(DadosPacote* pacote : resultados){
            cout << pacote->pacote << endl << configs->nlind << pacote->nome << endl << configs->nlind << pacote->descrição << endl << endl;;
        }
        return true;
    }
    return false;
}

bool GerenciadorPacotes::instalarPacotes(){
    for(string pacoteNome : configs->nomes){
        bool instalado = false;
        for(RemoteRepoConfig* repoconfig : configs->reposglobais){
            for(DadosPacote* pacote : repoconfig->pacotes){
                if(pacote->pacote==pacoteNome){
                    cout << configs->stringsidioma->BAIXANDO[0] << pacote->nome << "..." << endl;
                    string tempPath = Utilitarios::obterPastaTemporaria()+"/"+filesystem::path(pacote->endereço).filename().string();
                    //Depurando, remover depois
                    cout << pacote->endereço << endl;
                    if(repomanager->baixar(pacote->endereço, tempPath, true)){
                        cout << configs->nlind << configs->stringsidioma->INSTALANDO[0] << pacote->nome << "..." << endl;
                        string installCmd = "pm install -r \""+tempPath+"\"";
                        int ret = system(installCmd.c_str());
                        if(ret==0){
                            cout << configs->stringsidioma->INSTALADO[0] << pacote->nome << "!" << endl;
                            instalado=true;
                        }else{
                            cerr << configs->nlinderr << configs->stringsidioma->ERRO_INSTALAR[0] << pacote->nome << configs->stringsidioma->ERRO_INSTALAR[1] << endl;
                        }
                        filesystem::remove(tempPath);
                    }else{
                        cerr << configs->nlinderr << configs->stringsidioma->ERRO_BAIXAR[0] << pacote->nome << configs->stringsidioma->ERRO_BAIXAR[1] << endl;
                    }
                }
            }
        }
        if(!instalado){
            cerr << configs->stringsidioma->PACOTE_N_ENCONTRADO[0] << pacoteNome << configs->stringsidioma->PACOTE_N_ENCONTRADO[1] << endl;
        }
    }
    return true;
}