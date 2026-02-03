#include "gerenciador_pacotes.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include "apkm.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <lib/zip.h>
using namespace std;
using namespace Utilitarios;

void GerenciadorPacotes::pesquisar(){
    vector<DadosPacote*> resultados;
    string pesqcmresult;
    for(string pesquisa : configs->nomes){
        bool encontrado = false;
        for(RemoteRepoConfig* repoconfig : configs->reposglobais){
            for(DadosPacote* pacote : repoconfig->pacotes){
                if(
                    (search_match(pacote->descrição, pesquisa) ||
                    search_match(pacote->nome, pesquisa) ||
                    search_match(pacote->pacote, pesquisa)) && (checarCompatibilidade(pacote->arquiteturas) || configs->exibirIncompatíveis)
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
        mostrarListaResultados(resultados, configs);
    }

    for(AddOn* addon : configs->addonsdinamicos){
        string pesquisaTermo;
        for(string pesquisa : configs->nomes){
            if(pesquisaTermo==""){
                pesquisaTermo=pesquisa;
            }else pesquisaTermo.append(','+pesquisa);
        }
        vector<DadosPacote*> pacotes = addon->Buscar(pesquisaTermo);
        cout << pacotes.size() << " resultados em " << addon->config->nome << "." << endl;
        mostrarListaResultados(pacotes, configs);
    }
}

void GerenciadorPacotes::mostrarListaResultados(vector<DadosPacote*> pacotes, Config* configs){
    for(DadosPacote* pacote : pacotes){
        cout << gerarLinhaSeparadora() << endl;
        vector<string>* DADOS_PACOTE = &configs->stringsidioma->DADOS_PACOTE;
        if(configs->terminalColor){
            cout << BOLDCYAN << (*DADOS_PACOTE)[0] << BOLDGREEN << pacote->pacote << RESET;
            if(!checarCompatibilidade(pacote->arquiteturas)){
                string listaArchs;
                for(string arch : pacote->arquiteturas){
                    if(listaArchs==""){
                        listaArchs=arch;
                    }else listaArchs.append("/"+arch);
                }
                cout << " " << BOLDRED << configs->stringsidioma->PAC_INCOMPATIVEL[0] << listaArchs << configs->stringsidioma->PAC_INCOMPATIVEL[1] << RESET << endl;
            }else{
                cout << endl;
            }
            NLINDINFO((*DADOS_PACOTE)[1]+pacote->nome);
            NLINDINFO((*DADOS_PACOTE)[2]+pacote->descrição);
            NLINDINFO((*DADOS_PACOTE)[3]+pacote->origem);
        }else{
            cout << (*DADOS_PACOTE)[0] << pacote->pacote << endl;
            if(!checarCompatibilidade(pacote->arquiteturas)){
                string listaArchs;
                for(string arch : pacote->arquiteturas){
                    if(listaArchs==""){
                        listaArchs=arch;
                    }else listaArchs.append("/"+arch);
                }
                cout << " " << configs->stringsidioma->PAC_INCOMPATIVEL[0] << listaArchs << configs->stringsidioma->PAC_INCOMPATIVEL[1] << endl;
            }else{
                cout << endl;
            }
            NCNLINDINFO((*DADOS_PACOTE)[1]+pacote->nome);
            NCNLINDINFO((*DADOS_PACOTE)[2]+pacote->descrição);
            NCNLINDINFO((*DADOS_PACOTE)[3]+pacote->origem);
        }
    }
    cout << endl;
}

//Verifica e instala os pacotes listados em configs->nomes
bool GerenciadorPacotes::prepararInstalarPacotes(){
    for(string pacoteNome : configs->nomes){
        bool encontrado = false;
        for(RemoteRepoConfig* repoconfig : configs->reposglobais){
            for(DadosPacote* pacote : repoconfig->pacotes){
                if(pacote->pacote==pacoteNome){
                    if(instalarPacote(pacote, repoconfig))
                    encontrado=true;
                }
            }
        }
        if(!encontrado)
        for(AddOn* addon : configs->addonsdinamicos){
            RemoteRepoConfig* repoconfig = addon->ObterPacote(pacoteNome);
            if(repoconfig && instalarPacote(repoconfig->pacotes[0], repoconfig)){
                encontrado=true;
            }
        }
        if(!encontrado){
            cerr << configs->stringsidioma->PACOTE_N_ENCONTRADO[0] << pacoteNome << configs->stringsidioma->PACOTE_N_ENCONTRADO[1] << endl;
        }
    }
    return true;
}

bool GerenciadorPacotes::instalarPacote(DadosPacote* pacote, RemoteRepoConfig* repoconfig){
    cout << configs->stringsidioma->BAIXANDO[0] << pacote->nome << "..." << endl;
    string tempPath = obterPastaTemporaria()+"/downloading";
    filesystem::remove_all(tempPath);
    filesystem::create_directories(tempPath);
    string tempFile = tempPath+"/"+pacote->pacote+".apk";
    Tools tools = Tools(repoconfig->pinned_hashes, pacote->endereço, configs, configs->ssl);
    if(repomanager->baixarArquivo(pacote->endereço, tempFile, true, tools)){
        APKManifesto* manifesto = verificarApk(tempFile, pacote);
        if(GerenciadorPacotes::VerificarIntegridadePacote(tempFile, pacote->sha256sum)){
            if( manifesto==nullptr || !manifesto->manifestoValido ){
                NLINDERR(configs->stringsidioma->ERRO_INSTALAR[0]+pacote->nome+" (APK verification failed)"+configs->stringsidioma->ERRO_INSTALAR[1]);
                filesystem::remove_all(tempPath);
                delete manifesto;
                return false;
            }

            //Exibindo permissões do APK
            bool instalarApp = false;
            if(!configs->assumirSim){
                NLIND(configs->stringsidioma->QUEST_INSTALAR_APP[0]+pacote->nome+configs->stringsidioma->QUEST_INSTALAR_APP[1]);
                if(manifesto->permissions.size()>0){
                    NLIND(configs->stringsidioma->PERMISS_REQUERIDAS[0]);
                    cout << gerarLinhaSeparadora() << endl;
                    for(string perm : manifesto->permissions){
                        NLINDINFO(" - "+configs->stringsidioma->obterPermissãoTexto(perm));
                    }
                    cout << gerarLinhaSeparadora() << endl;
                    NLINDINPUT(configs->stringsidioma->QUEST_INSTALAR_APP[2]);
                    string resposta;
                    getline(cin, resposta);
                    if(resposta!="s" && resposta!="S" && resposta!="y" && resposta!="Y"){
                        NLIND(configs->stringsidioma->INSTAL_CANCELADA[0]+pacote->nome+configs->stringsidioma->INSTAL_CANCELADA[1]);
                        instalarApp = false;
                    }else{
                        instalarApp = true;
                    }
                }
            }
            if(configs->assumirSim || instalarApp){
                NLIND(configs->stringsidioma->INSTALANDO[0]+pacote->nome+"...");
                //Comando de instalação via pm
                string installCmd = "pm install -r \""+tempFile+"\"";
                FILE* pipe = popen(installCmd.c_str(), "r");
                char buffer[128];
                while (fgets(buffer, 128, pipe)) {
                    NLIND(buffer);
                }
                int ret = pclose(pipe);
                if(ret==0){
                    cout << configs->stringsidioma->INSTALADO[0] << pacote->nome << configs->stringsidioma->INSTALADO[1] << endl;
                    return true;
                }else{
                    NLINDERR(configs->stringsidioma->ERRO_INSTALAR[0]+pacote->nome+configs->stringsidioma->ERRO_INSTALAR[1]);
                }
            }
        }else{
            NLINDERR(configs->stringsidioma->ERRO_INSTALAR[0]+pacote->nome+" (SHA256 mismatch)"+configs->stringsidioma->ERRO_INSTALAR[1]);
        }
        filesystem::remove_all(tempPath);
    }else{
        NLINDERR(configs->stringsidioma->ERRO_BAIXAR[0]+pacote->nome+configs->stringsidioma->ERRO_BAIXAR[1]);
        return false;
    }
    return false;
}

//Verifica a integridade do pacote comparando o hash SHA256
bool GerenciadorPacotes::VerificarIntegridadePacote(std::string arquivoPath, std::string expectedSha256){
    std::string arquivoSha256 = calcularHashArquivo(arquivoPath, EVP_sha256());
    return arquivoSha256 == expectedSha256;
}

//Calcula o hash de um arquivo usando o algoritmo especificado
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

//Desinstala os pacotes listados em configs->nomes
bool GerenciadorPacotes::desinstalarPacotes(){
    for(string pacoteNome : configs->nomes){
        cout << configs->stringsidioma->DESINSTALANDO[0] << pacoteNome << "..." << endl;
        //Comando de remoção via pm
        std::string removerCmd = "pm uninstall \""+pacoteNome+"\"";
        FILE* pipe = popen(removerCmd.c_str(), "r");
        char buffer[128];
        while (fgets(buffer, 128, pipe)) {
            NLIND(buffer);
        }
        int ret = pclose(pipe);
        if(ret==0){
            cout << configs->stringsidioma->DESINSTALADO[0] << pacoteNome << configs->stringsidioma->DESINSTALADO[1] << endl;
            return true;
        }else{
            NLINDERR(configs->stringsidioma->ERRO_DESINSTALAR[0]+pacoteNome+configs->stringsidioma->ERRO_DESINSTALAR[1]);
        }
    }
    return true;
}

//Parser de AndroidManifest.xml para extrair informações básicas do APK
APKManifesto* GerenciadorPacotes::verificarApk(filesystem::path apkPath, DadosPacote* pacote){
    // Usamos smart pointer ou garantimos a limpeza para evitar leak
    APKManifesto* manifesto = new APKManifesto();
    manifesto->manifestoValido = false;
    int err = 0;
    zip *z = zip_open(apkPath.c_str(), 0, &err);
    if (!z) {
        delete manifesto;
        return nullptr;
    }

    zip_stat_t st;
    zip_stat_init(&st);
    if (zip_stat(z, "AndroidManifest.xml", 0, &st) == 0) {
        std::vector<uint8_t> manifestBuffer(st.size);
        zip_file *f = zip_fopen(z, "AndroidManifest.xml", 0);
        zip_fread(f, manifestBuffer.data(), st.size);
        zip_fclose(f);

        auto strings = extrairStringsAxml(manifestBuffer);
    
        bool pacoteEncontrado = false;

        for (const auto& s : strings) {
            if (s == pacote->pacote) {
                pacoteEncontrado = true;
                manifesto->packageName = s;
            }
            if (s.find("android.permission.") != std::string::npos) {
                manifesto->permissions.push_back(s);
            }
        }

        if (!pacoteEncontrado) {
            zip_close(z);
            delete manifesto;
            return nullptr;
        }

        // Se chegamos aqui, o pacote é válido
        manifesto->manifestoValido = true;
    }
    
    zip_close(z);
    
    if (manifesto->manifestoValido) return manifesto;
    
    delete manifesto;
    return nullptr;
}

// Função para extrair strings do buffer binário
std::vector<std::string> GerenciadorPacotes::extrairStringsAxml(const std::vector<uint8_t>& buffer) {
    std::vector<std::string> strings;
    if (buffer.size() < 8) return strings;

    // O String Pool geralmente é o primeiro chunk após o cabeçalho do arquivo
    // Offset 8: Início do String Pool Chunk
    const uint8_t* ptr = buffer.data() + 8;
    AxmlChunkHeader* poolHeader = (AxmlChunkHeader*)ptr;

    if (poolHeader->type == 0x0001) { // RES_STRING_POOL_TYPE
        uint32_t stringCount = *(uint32_t*)(ptr + 8);
        uint32_t stringOffset = *(uint32_t*)(ptr + 20);
        const uint32_t* indices = (const uint32_t*)(ptr + 28);
        const uint8_t* poolData = ptr + stringOffset;

        for (uint32_t i = 0; i < stringCount; ++i) {
            const uint8_t* sPtr = poolData + indices[i];
            // No AXML, strings costumam ter 2 bytes de tamanho antes do texto
            uint16_t u16len = *(uint16_t*)sPtr;
            std::string s;
            // Pula os bytes de tamanho e lê os caracteres (simplificado para ASCII)
            for (int j = 0; j < u16len; j++) {
                char c = sPtr[2 + (j * 2)]; // UTF-16LE jump
                if (c > 0) s += c;
            }
            strings.push_back(s);
        }
    }
    return strings;
}