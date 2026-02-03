#include <locale>
#include <iostream>
#include <vector>
#include <fstream>
#include <sys/system_properties.h>
#include "idiomas.hpp"
#include "apkm.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

//Construtor que carrega as strings do arquivo de idioma JSON
Strings::Strings(Config* config){
    //Tenta carregar o idioma do sistema, se falhar, carrega o inglês por padrão
    try{
        std::string idiomastr = Utilitarios::getProp("persist.sys.locale");
        //Carreganndo idioma da pasta de idiomas
        std::string idiomaFile = config->diretórioIdiomas+"/"+idiomastr+".json";
        if(!std::filesystem::exists(idiomaFile)){
            idiomaFile = config->diretórioIdiomas+"/en-US.json";
        }
        std::ifstream file(idiomaFile);
        json j = json::parse(file);
        auto stringsJson = j["strings"];
        CARREGANDO_REPOSITORIOS = stringsJson["CARREGANDO_REPOSITORIOS"].get<std::vector<std::string>>();
        ERRO_REPOS_N_ENCONTRADOS = stringsJson["ERRO_REPOS_N_ENCONTRADOS"].get<std::vector<std::string>>();
        SIM = stringsJson["SIM"].get<std::vector<std::string>>();
        NAO = stringsJson["NAO"].get<std::vector<std::string>>();
        USANDO_SSL = stringsJson["USANDO_SSL"].get<std::vector<std::string>>();
        ERRO_OBTER_CERTIFICADO = stringsJson["ERRO_OBTER_CERTIFICADO"].get<std::vector<std::string>>();
        CERT_NAO_CONFIAVEL = stringsJson["CERT_NAO_CONFIAVEL"].get<std::vector<std::string>>();
        ERRO_HASHS_DESCONHECIDOS = stringsJson["ERRO_HASHS_DESCONHECIDOS"].get<std::vector<std::string>>();
        VERIFICANDO_REPOSITORIO = stringsJson["VERIFICANDO_REPOSITORIO"].get<std::vector<std::string>>();
        PRONTO = stringsJson["PRONTO"].get<std::vector<std::string>>();
        ERRO_COM_INVALIDO = stringsJson["ERRO_COM_INVALIDO"].get<std::vector<std::string>>();
        AJUDA = stringsJson["AJUDA"].get<std::vector<std::string>>();
        REPOSITORIO_INVALIDO = stringsJson["REPOSITORIO_INVALIDO"].get<std::vector<std::string>>();
        ADICIONANDO_REPOSITÒRIOS = stringsJson["ADICIONANDO_REPOSITÒRIOS"].get<std::vector<std::string>>();
        REPOSITORIO_ADICIONADO = stringsJson["REPOSITORIO_ADICIONADO"].get<std::vector<std::string>>();
        REPOSITORIO_REMOVIDO = stringsJson["REPOSITORIO_REMOVIDO"].get<std::vector<std::string>>();
        REPOSITORIO_N_ENCONTRADO = stringsJson["REPOSITORIO_N_ENCONTRADO"].get<std::vector<std::string>>();
        NENHUM_REPO_ENCONTRADO = stringsJson["NENHU_REPO_ENCONTRADO"].get<std::vector<std::string>>();
        MOSTR_RESULTS = stringsJson["MOSTR_RESULTS"].get<std::vector<std::string>>();
        BAIXANDO = stringsJson["BAIXANDO"].get<std::vector<std::string>>();
        INSTALANDO = stringsJson["INSTALANDO"].get<std::vector<std::string>>();
        INSTALADO = stringsJson["INSTALADO"].get<std::vector<std::string>>();
        ERRO_INSTALAR = stringsJson["ERRO_INSTALAR"].get<std::vector<std::string>>();
        ERRO_BAIXAR = stringsJson["ERRO_BAIXAR"].get<std::vector<std::string>>();
        PACOTE_N_ENCONTRADO = stringsJson["PACOTE_N_ENCONTRADO"].get<std::vector<std::string>>();
        DESINSTALADO = stringsJson["DESINSTALADO"].get<std::vector<std::string>>();
        ERRO_DESINSTALAR = stringsJson["ERRO_DESINSTALAR"].get<std::vector<std::string>>();
        DESINSTALANDO = stringsJson["DESINSTALANDO"].get<std::vector<std::string>>();
        PERMISS_REQUERIDAS = stringsJson["PERMISS_REQUERIDAS"].get<std::vector<std::string>>();
        QUEST_INSTALAR_APP = stringsJson["QUEST_INSTALAR_APP"].get<std::vector<std::string>>();
        INSTAL_CANCELADA = stringsJson["INSTAL_CANCELADA"].get<std::vector<std::string>>();
        ADDON_INFOS = stringsJson["ADDON_INFOS"].get<std::vector<std::string>>();
        DADOS_PACOTE = stringsJson["DADOS_PACOTE"].get<std::vector<std::string>>();
        PAC_INCOMPATIVEL = stringsJson["PAC_INCOMPATIVEL"].get<std::vector<std::string>>();
        
        auto permissoesJson = j["permissoes_texto"];
        for (auto& [key, value] : permissoesJson.items()) {
            permissõesTexto[key] = value.get<std::string>();
        }
    }catch(...){
        //Inglês padrão embutido
        CARREGANDO_REPOSITORIOS = { "Loading repository list" };
        ERRO_REPOS_N_ENCONTRADOS = { "Error listing repositories, downloading original repositories" };
        SIM = { "Yes" };
        NAO = { "No" };
        USANDO_SSL = { "Using SSL" };
        ERRO_OBTER_CERTIFICADO = { "Could not obtain the certificate." };
        CERT_NAO_CONFIAVEL = { "The certificate is not trustworthy" };
        ERRO_HASHS_DESCONHECIDOS = { "No hashes provided by the server match the repository, remove with \"","\", or try updating the repository with \",\" assuming the repository is valid." };
        VERIFICANDO_REPOSITORIO = { "Verifying repository " };
        PRONTO = { "Done." };
        ERRO_COM_INVALIDO = { "The command \"", "\" is not a valid command.\n\n" };
        AJUDA = { "$$BIN Usage: $$BIN  [COMMAND]\n\nCommands:\n\n$$BIN update\n    Updates repositories and apps to the latest version available online\n\n$$BIN --add-repo <url>\n    Adds a repository, be careful this command does not guarantee that the repository at the url is safe.\n    example: $$BIN --add-repo https://raw.githubusercontent.com/Eryck2486/apkm/refs/heads/main/exemplo_repositorio_root/\n\n$$BIN rm-repo <repository-name>\n    Removes the repository\n\n$$BIN list/$$BIN list repos\n    lists the available repositories\n$$BIN list addons\n    lists the available addons\n\n $$BIN search <term>\n    searches for terms in all repositories\n\n$$BIN install <package>\n    looks for the latest version of the app in the repositories and installs it.\n    example: $$BIN install com.termux\n\n$$BIN uninstall <package>\n    uninstalls the package from the device\n\nAPKM is an open source tool, the official repository is available at https://github.com/Eryck2486/apkm/\n" };
        REPOSITORIO_INVALIDO = { "Error: The provided repository address is not a valid repository or does not contain a valid \"index.json\" file." };
        ADICIONANDO_REPOSITÒRIOS = { "Are you sure you want to add the repository \"","\" to your list of repositories?\nSecurity is not guaranteed in the addition process, confirm with Enter or cancel with Ctrl+C." };
        REPOSITORIO_ADICIONADO = { "Repository added successfully!." };
        REPOSITORIO_REMOVIDO = { "Repository \"","\" removed successfully!." };
        REPOSITORIO_N_ENCONTRADO = { "The repository \"","\" was not found." };
        NENHUM_REPO_ENCONTRADO = { "No repository found." };
        MOSTR_RESULTS = { "The following results were found for: " };
        BAIXANDO = { "Downloading package " };
        INSTALANDO = { "Installing package " };
        INSTALADO = { "App \"","\" installed successfully!." };
        ERRO_INSTALAR = { "Error installing app \"","\"." };
        ERRO_BAIXAR = { "Error downloading app \"","\"." };
        PACOTE_N_ENCONTRADO = { "The package \"","\" was not found in any available repository." };
        DESINSTALADO = { "App \"","\" uninstalled successfully!." };
        ERRO_DESINSTALAR = { "Error uninstalling app \"","\"." };
        DESINSTALANDO = { "Uninstalling package..." };
        PERMISS_REQUERIDAS = { "Required Permissions:" };
        QUEST_INSTALAR_APP = { "You are about to install the app \"","\""," proceed with the installation? (y/n): " };
        INSTAL_CANCELADA = { "Installation of app \"","\" canceled by user." };

        std::unordered_map<std::string, std::string> defaultPermissoes = {
            {"android.permission.INTERNET", "Access the internet."},
            {"android.permission.ACCESS_NETWORK_STATE", "Read network information."},
            {"android.permission.READ_EXTERNAL_STORAGE", "Read external storage data."},
            {"android.permission.WRITE_EXTERNAL_STORAGE", "Write data to external storage."},
            {"android.permission.CAMERA", "Access the device camera."},
            {"android.permission.RECORD_AUDIO", "Record audio using the device microphone."},
            {"android.permission.ACCESS_FINE_LOCATION", "Access precise device location."},
            {"android.permission.ACCESS_COARSE_LOCATION", "Access approximate device location."},
            {"android.permission.READ_CONTACTS", "Read contacts stored on the device."},
            {"android.permission.WRITE_CONTACTS", "Modify contacts stored on the device."},
            {"android.permission.CALL_PHONE", "Make phone calls directly from the app."},
            {"android.permission.SEND_SMS", "Send SMS messages."},
            {"android.permission.READ_SMS", "Read received SMS messages."},
            {"android.permission.RECEIVE_SMS", "Receive SMS messages."},
            {"android.permission.READ_CALENDAR", "Read calendar events."},
            {"android.permission.WRITE_CALENDAR", "Modify calendar events."}
            // Add more default permissions as needed
        };
        permissõesTexto = defaultPermissoes;
    }
}

//Obtém o texto amigável para uma permissão específica
std::string Strings::obterPermissãoTexto(std::string perm){
    if(permissõesTexto.find(perm) != permissõesTexto.end()){
        return permissõesTexto[perm];
    }else{
        return perm;
    }
}

