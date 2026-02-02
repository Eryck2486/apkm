#include "gerenciador-pacotes.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include "utilitarios.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <lib/zip.h>
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
        bool encontrado = false;
        for(RemoteRepoConfig* repoconfig : configs->reposglobais){
            for(DadosPacote* pacote : repoconfig->pacotes){
                if(pacote->pacote==pacoteNome){
                    encontrado=true;
                    cout << configs->stringsidioma->BAIXANDO[0] << pacote->nome << "..." << endl;
                    string tempPath = Utilitarios::obterPastaTemporaria()+"/downloading";
                    filesystem::remove_all(tempPath);
                    filesystem::create_directories(tempPath);
                    string tempFile = tempPath+"/"+pacote->pacote+".apk";
                    if(repomanager->baixarApk(pacote->endereço, tempFile, true, repoconfig)){
                        if(GerenciadorPacotes::VerificarIntegridadePacote(tempFile, pacote->sha256sum)){
                            cout << configs->nlind << configs->stringsidioma->INSTALANDO[0] << pacote->nome << "..." << endl;
                            //Comando de instalação via pm
                            string installCmd = "pm install -r \""+tempFile+"\"";
                            FILE* pipe = popen(installCmd.c_str(), "r");
                            char buffer[128];
                            while (fgets(buffer, 128, pipe)) {
                                std::cout << configs->nlind << buffer;
                            }
                            int ret = pclose(pipe);
                            if(ret==0){
                                cout << configs->stringsidioma->INSTALADO[0] << pacote->nome << configs->stringsidioma->INSTALADO[1] << endl;
                                return true;
                            }else{
                                cerr << configs->nlinderr << configs->stringsidioma->ERRO_INSTALAR[0] << pacote->nome << configs->stringsidioma->ERRO_INSTALAR[1] << endl;
                            }
                        }else{
                            cerr << configs->nlinderr << configs->stringsidioma->ERRO_INSTALAR[0] << pacote->nome << " (SHA256 mismatch)" << configs->stringsidioma->ERRO_INSTALAR[1] << endl;
                        }
                        //Testando extração do APK
                        //bool apkOk =
                        verificarApk(tempFile, pacote);
                        //filesystem::remove_all(tempPath);
                    }else{
                        cerr << configs->nlinderr << configs->stringsidioma->ERRO_BAIXAR[0] << pacote->nome << configs->stringsidioma->ERRO_BAIXAR[1] << endl;
                        return false;
                    }
                }
            }
        }
        if(!encontrado){
            cerr << configs->stringsidioma->PACOTE_N_ENCONTRADO[0] << pacoteNome << configs->stringsidioma->PACOTE_N_ENCONTRADO[1] << endl;
        }
    }
    return true;
}

bool GerenciadorPacotes::VerificarIntegridadePacote(std::string arquivoPath, std::string expectedSha256){
    std::string arquivoSha256 = calcularHashArquivo(arquivoPath, EVP_sha256());
    return arquivoSha256 == expectedSha256;
}

std::string GerenciadorPacotes::calcularHashArquivo(const std::string& caminho, const EVP_MD* algoritmo) {
    std::ifstream arquivo(caminho, std::ios::binary);
    if (!arquivo.is_open()) return "";

    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, algoritmo, NULL);

    char buffer[8192]; // Lê 8KB por vez
    while (arquivo.read(buffer, sizeof(buffer)) || arquivo.gcount() > 0) {
        EVP_DigestUpdate(context, buffer, arquivo.gcount());
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    EVP_DigestFinal_ex(context, hash, &length);
    EVP_MD_CTX_free(context);

    // Converte os bytes para string Hexadecimal
    std::stringstream ss;
    for (unsigned int i = 0; i < length; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool GerenciadorPacotes::desinstalarPacotes(){
    for(string pacoteNome : configs->nomes){
        cout << configs->stringsidioma->DESINSTALANDO[0] << pacoteNome << "..." << endl;
        //Comando de remoção via pm
        std::string removerCmd = "pm uninstall \""+pacoteNome+"\"";
        FILE* pipe = popen(removerCmd.c_str(), "r");
        char buffer[128];
        while (fgets(buffer, 128, pipe)) {
            std::cout << configs->nlind << buffer;
        }
        int ret = pclose(pipe);
        if(ret==0){
            cout << configs->stringsidioma->DESINSTALADO[0] << pacoteNome << configs->stringsidioma->DESINSTALADO[1] << endl;
            return true;
        }else{
            cerr << configs->nlinderr << configs->stringsidioma->ERRO_DESINSTALAR[0] << pacoteNome << configs->stringsidioma->ERRO_DESINSTALAR[1] << endl;
        }
    }
    return true;
}

bool GerenciadorPacotes::verificarApk(filesystem::path apkPath, DadosPacote* pacote){
    if(GerenciadorPacotes::VerificarIntegridadePacote(apkPath, pacote->sha256sum)) {
        int err = 0;
        zip *z = zip_open(apkPath.c_str(), 0, &err);
        if (z) {
            zip_stat_t st;
            zip_stat_init(&st);
            if (zip_stat(z, "AndroidManifest.xml", 0, &st) == 0) {
                // O arquivo existe, podemos ler os bytes
                std::vector<uint8_t> manifest(st.size);
                zip_file *f = zip_fopen(z, "AndroidManifest.xml", 0);
                zip_fread(f, manifest.data(), st.size);
                zip_fclose(f);
                
                // Aqui você faria a verificação do buffer binário 'manifest'
                cout << configs->nlind << "Manifesto extraído (" << st.size << " bytes)" << endl;
            }
            zip_close(z);
            return true;
        }else {
            cerr << configs->nlinderr << "Falha na verificação de integridade do APK." << endl;
        }
    }
    return false;
} 
