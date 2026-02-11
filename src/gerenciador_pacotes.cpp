#include "gerenciador_pacotes.hpp"
#include "apkm_packages_manager.hpp"
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
#include <thread>
#include <unordered_map>
#include <regex>
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

    //Mostra os resultados dos repositórios
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
        vector<DadosPacote*> pacotesFiltrados = vector<DadosPacote*>();
        //Filtra os pacotes para exibir somente os compatíveis
        if(configs->exibirIncompatíveis){
            pacotesFiltrados=pacotes;
        }else for(DadosPacote* pacote : addon->Buscar(pesquisaTermo)){
            if(checarCompatibilidade(pacote->arquiteturas)){
                pacotesFiltrados.push_back(pacote);
            }
        }
        if(pacotes.size()>0) mostrarListaResultados(pacotesFiltrados, configs);
    }
}

void GerenciadorPacotes::mostrarListaResultados(vector<DadosPacote*> pacotes, Config* configs){
    for(DadosPacote* pacote : pacotes){
        if(configs->formatoJSON){
            cout << pacote->toJson() << endl;
        }else{
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
                NLINDINFO((*DADOS_PACOTE)[3]+pacote->versão);
                NLINDINFO((*DADOS_PACOTE)[4]+pacote->origem);
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
                NCNLINDINFO((*DADOS_PACOTE)[3]+pacote->versão);
                NCNLINDINFO((*DADOS_PACOTE)[4]+pacote->origem);
            }
        }
    }
    cout << endl;
}

//Verifica e instala os pacotes listados em configs->nomes
bool GerenciadorPacotes::prepararInstalarPacotes(){
    for(string pacoteNome : configs->nomes){
        bool encontrado = false;
        if(pacoteNome.find(":") == std::string::npos){
            for(RemoteRepoConfig* repoconfig : configs->reposglobais){
                for(DadosPacote* pacote : repoconfig->pacotes){
                    if(pacote->pacote==pacoteNome){
                        if(instalarPacote(pacote, repoconfig))
                        encontrado=true;
                    }
                }
            }
            if(!encontrado){
                if(configs->formatoJSON){
                    cout << "{\"status\":\"fail\", \"message\":\"Package not found: "+pacoteNome+"\"}" << endl;
                }else{
                    cerr << configs->stringsidioma->PACOTE_N_ENCONTRADO[0] << pacoteNome << configs->stringsidioma->PACOTE_N_ENCONTRADO[1] << endl;
                }
            }
        }else{
            //Testando busca de prefixo de AddOn
            vector<string> pacoteData = stringSplit(&pacoteNome, ':');
            string addonPrefix = pacoteData[0];
            string pacoteNome = pacoteData[1];
            for(AddOn* addon : configs->addonsdinamicos){
                if(addon->config->prefix==addonPrefix){
                    vector<string> pacoteLocal = addon->getPackage(pacoteNome);
                    //Movendo APK para pasta temporária e instalando
                    filesystem::path tempPath = obterPastaTemporaria()+"/"+pacoteNome+".apk";
                    if(filesystem::exists(tempPath)){
                        filesystem::remove(tempPath);
                    }
                    filesystem::copy_file(pacoteLocal[0], tempPath);
                    pacoteLocal[0]=tempPath;
                    if(pacoteLocal.size()>0 && apkInstaller(pacoteLocal[0], pacoteLocal[1], configs)){
                        encontrado=true;
                    }
                }
            }
        }
        if(!encontrado){
            if(configs->formatoJSON){
                cout << "{\"status\":\"fail\", \"message\":\"Package not found: "+pacoteNome+"\"}" << endl;
            }else{
                cerr << configs->stringsidioma->PACOTE_N_ENCONTRADO[0];
                cerr << pacoteNome;
                cerr << configs->stringsidioma->PACOTE_N_ENCONTRADO[1] << endl;
            }
        }
    }
    return true;
}

bool GerenciadorPacotes::instalarPacote(DadosPacote* pacote, RemoteRepoConfig* repoconfig){
    if(!configs->formatoJSON) cout << configs->stringsidioma->BAIXANDO[0] << pacote->nome << "..." << endl;
    string tempPath = obterPastaTemporaria()+"/downloading";
    filesystem::remove_all(tempPath);
    filesystem::create_directories(tempPath);
    string tempFile = tempPath+"/"+pacote->pacote+".apk";
    Tools tools = Tools(repoconfig->pinned_hashes, pacote->endereço, configs, configs->ssl);
    if(repomanager->baixarArquivo(pacote->endereço, tempFile, true, tools)){
        if(GerenciadorPacotes::VerificarIntegridadePacote(tempFile, pacote->sha256sum)){
            return apkInstaller(tempFile, pacote->pacote, configs);
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

bool GerenciadorPacotes::apkInstaller(std::filesystem::path apkPath, std::string pacote, Config* configs){
    cout << "Verificando integridade do pacote..." << endl;
    APKManifesto* manifesto = verificarApk(apkPath, pacote, configs);
    if( manifesto==nullptr || !manifesto->manifestoValido ){
        string manifestoErro;
        if(manifesto==nullptr){
            manifestoErro = "APK verification failed: Manifest not found or unreadable.";
        }else if(!manifesto->manifestoValido){
            manifestoErro = "APK verification failed: Package name not found in manifest.";
        }
        if(configs->formatoJSON){
            cout << "{\"status\":\"fail\", \"message\":\"APK verification failed for "+pacote+"\": \"" << manifestoErro << "\"}" << endl;
        }else{
            NLINDERR(configs->stringsidioma->ERRO_INSTALAR[0]+pacote+" (APK verification failed)"+configs->stringsidioma->ERRO_INSTALAR[1]+": "+manifestoErro);
            filesystem::remove_all(apkPath);
            delete manifesto;
        }
        return false;
    }

    //Exibindo permissões do APK
    bool instalarApp = false;
    if(!configs->assumirSim && !configs->formatoJSON){
        NLIND(configs->stringsidioma->QUEST_INSTALAR_APP[0]+pacote+configs->stringsidioma->QUEST_INSTALAR_APP[1]);
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
                NLIND(configs->stringsidioma->INSTAL_CANCELADA[0]+pacote+configs->stringsidioma->INSTAL_CANCELADA[1]);
                return true;
            }else{
                instalarApp = true;
            }
        }
    }
    if((configs->assumirSim || configs->formatoJSON) || instalarApp){
        if(!configs->formatoJSON) NLIND(configs->stringsidioma->INSTALANDO[0]+pacote+"...");
        else cout << "{\"status\":\"installing\", \"message\":\""+configs->stringsidioma->INSTALANDO[0]+pacote+"...\"}" << endl;
        //Comando de instalação via pm
        string installCmd = "pm install -r \"";
        installCmd.append(apkPath.c_str());
        installCmd.append("\"");
        FILE* pipe = popen(installCmd.c_str(), "r");
        char buffer[128];
        string output;
        while (fgets(buffer, 128, pipe)) {
            output += buffer;
            if(!configs->formatoJSON) NLIND(output);
        }
        int ret = pclose(pipe);
        if(ret==0){
            if(configs->formatoJSON){
                cout << "{\"status\":\"success\", \"message\":\"Package \""+pacote+"\" installed successfully: "+output+"\"}" << endl;
            }else{
                NLIND(configs->stringsidioma->INSTALADO[0]+pacote+configs->stringsidioma->INSTALADO[1]);
            }
            return true;
        }else{
            if(configs->formatoJSON){
                cout << "{\"status\":\"fail\", \"message\":\"Failed to install package \""+pacote+"\": "+output+"\"}" << endl;
            }else{
                NLINDERR(configs->stringsidioma->ERRO_INSTALAR[0]+pacote+configs->stringsidioma->ERRO_INSTALAR[1]);
            }
            return false;
        }
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
        string resultCmd = executarComandoShell(removerCmd);
        if(stringContains(&resultCmd, "Success")){
            string info = configs->stringsidioma->DESINSTALADO[0] + pacoteNome + configs->stringsidioma->DESINSTALADO[1];
            if(!configs->formatoJSON){
                cout << info << endl;
            }else{
                printInfo("INFO", info);
            }
        }else{
            if(!configs->formatoJSON) NLINDERR(configs->stringsidioma->ERRO_DESINSTALAR[0]+pacoteNome+configs->stringsidioma->ERRO_DESINSTALAR[1]);
        }
    }
    return true;
}

//Parser de AndroidManifest.xml para extrair informações básicas do APK
APKManifesto* GerenciadorPacotes::verificarApk(filesystem::path apkPath, string pacote, Config* configs){
    // Usamos smart pointer ou garantimos a limpeza para evitar leak
    APKManifesto* manifesto = new APKManifesto();
    manifesto->manifestoValido = false;
    int err = 0;
    zip *z = zip_open(apkPath.c_str(), 0, &err);
    if (!z) {
        delete manifesto;
        cout << "Failed to open APK file: " << apkPath << endl;
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
            if (s == pacote) {
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

bool GerenciadorPacotes::upgradePacotes(){
    for(AddOn* addon : configs->addonsdinamicos){
        if(addon->getConfig()){
            if(addon->config->novaversao){
                vector<string> pacoteInfos = addon->getAddonUpdate();
                if(!apkInstaller(pacoteInfos[0], pacoteInfos[1], configs)){
                    //Retonar texto avisando da falha

                    //Aborta processo de atualizações para evitar mais erros
                    return false;
                }
            }
        }
    }
    //Map com pacote e versão do pacote
    unordered_map<string, int> pacotesInfos;
    Helper* helper = new Helper(configs);
    //Popula a lista de pacotes e obtem o JSON de pacotes instalados para a verificação via AddOn
    string retorno = obterVersõesPacotesInstalados(pacotesInfos, helper);
    delete helper;

    return true;
}

//Grava array de pacotes instalados e retorna o JSON bruto
string GerenciadorPacotes::obterVersõesPacotesInstalados(unordered_map<string, int> pacotesInfos, Helper* helper){
    vector<PackageInfo*> pacotes;
    string retorno = helper->getPackagesInfos(pacotes);
    for(PackageInfo* pacote : pacotes){
        pacotesInfos[pacote->getPackageName()]=stoi(pacote->getVersionCode());
        delete pacote;
    }
    return retorno;
}

//retorna id de pacote e versionCode
unordered_map<string, int> GerenciadorPacotes::obterUpdatesOnAddon(string pacotesJson, AddOn* addon){
    unordered_map<string, int> updates;
    return updates;
}
