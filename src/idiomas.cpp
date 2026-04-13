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
    json j;
    json stringsJson;
    try{
        std::string idiomastr = Utilitarios::getProp("persist.sys.locale");
        //Carreganndo idioma da pasta de idiomas
        std::string idiomaFile = config->diretórioIdiomas+"/"+idiomastr+".json";
        if(!std::filesystem::exists(idiomaFile)){
            idiomaFile = config->diretórioIdiomas+"/en-US.json";
        }
        std::ifstream file(idiomaFile);
        j = json::parse(file);
        stringsJson = j["strings"];
    }catch(...){
    }
    CARREGANDO_REPOSITORIOS = obterString(&stringsJson, "CARREGANDO_REPOSITORIOS");
    ERRO_REPOS_N_ENCONTRADOS = obterString(&stringsJson, "ERRO_REPOS_N_ENCONTRADOS");
    SIM = obterString(&stringsJson, "SIM");
    NAO = obterString(&stringsJson, "NAO");
    USANDO_SSL = obterString(&stringsJson, "USANDO_SSL");
    ERRO_OBTER_CERTIFICADO = obterString(&stringsJson, "ERRO_OBTER_CERTIFICADO");
    CERT_NAO_CONFIAVEL = obterString(&stringsJson, "CERT_NAO_CONFIAVEL");
    ERRO_HASHS_DESCONHECIDOS = obterString(&stringsJson, "ERRO_HASHS_DESCONHECIDOS");
    VERIFICANDO_REPOSITORIO = obterString(&stringsJson, "VERIFICANDO_REPOSITORIO");
    PRONTO = obterString(&stringsJson, "PRONTO");
    ERRO_COM_INVALIDO = obterString(&stringsJson, "ERRO_COM_INVALIDO");
    AJUDA = obterString(&stringsJson, "AJUDA");
    REPOSITORIO_INVALIDO = obterString(&stringsJson, "REPOSITORIO_INVALIDO");
    ADICIONANDO_REPOSITÒRIOS = obterString(&stringsJson, "ADICIONANDO_REPOSITÒRIOS");
    REPOSITORIO_ADICIONADO = obterString(&stringsJson, "REPOSITORIO_ADICIONADO");
    REPOSITORIO_REMOVIDO = obterString(&stringsJson, "REPOSITORIO_REMOVIDO");
    REPOSITORIO_N_ENCONTRADO = obterString(&stringsJson, "REPOSITORIO_N_ENCONTRADO");
    NENHUM_REPO_ENCONTRADO = obterString(&stringsJson, "NENHUM_REPO_ENCONTRADO");
    MOSTR_RESULTS = obterString(&stringsJson, "MOSTR_RESULTS");
    BAIXANDO = obterString(&stringsJson, "BAIXANDO");
    INSTALANDO = obterString(&stringsJson, "INSTALANDO");
    INSTALADO = obterString(&stringsJson, "INSTALADO");
    ERRO_INSTALAR = obterString(&stringsJson, "ERRO_INSTALAR");
    ERRO_BAIXAR = obterString(&stringsJson, "ERRO_BAIXAR");
    PACOTE_N_ENCONTRADO = obterString(&stringsJson, "PACOTE_N_ENCONTRADO");
    DESINSTALADO = obterString(&stringsJson, "DESINSTALADO");
    ERRO_DESINSTALAR = obterString(&stringsJson, "ERRO_DESINSTALAR");
    DESINSTALANDO = obterString(&stringsJson, "DESINSTALANDO");
    PERMISS_REQUERIDAS = obterString(&stringsJson, "PERMISS_REQUERIDAS");
    QUEST_INSTALAR_APP = obterString(&stringsJson, "QUEST_INSTALAR_APP");
    INSTAL_CANCELADA = obterString(&stringsJson, "INSTAL_CANCELADA");
    ADDON_INFOS = obterString(&stringsJson, "ADDON_INFOS");
    DADOS_PACOTE = obterString(&stringsJson, "DADOS_PACOTE");
    PAC_INCOMPATIVEL = obterString(&stringsJson, "PAC_INCOMPATIVEL");
    ERR_NO_REPOS = obterString(&stringsJson, "ERR_NO_REPOS");
    QUEST_ADDONS_UPGRADE = obterString(&stringsJson, "QUEST_ADDONS_UPGRADE");
    ADDON_ATUALIZADO = obterString(&stringsJson, "ADDON_ATUALIZADO");
    ADDON_N_ATUALIZADO = obterString(&stringsJson, "ADDON_N_ATUALIZADO");
    CHECKING_FOR_ADDON_UPGRADES = obterString(&stringsJson, "CHECKING_FOR_ADDON_UPGRADES");
    CHECKING_FOR_APP_UPGRADES = obterString(&stringsJson, "CHECKING_FOR_APP_UPGRADES");
    QUEST_PACOTES_UPGRADE = obterString(&stringsJson, "QUEST_PACOTES_UPGRADE");
    PACOTES_ATUALIZADOS = obterString(&stringsJson, "PACOTES_ATUALIZADOS");
    PACOTES_N_ATUALIZADOS = obterString(&stringsJson, "PACOTES_N_ATUALIZADOS");
    SEM_ATUALIZACOES = obterString(&stringsJson, "SEM_ATUALIZACOES");
    
    if(j!=json() && j.contains("permissoes_texto")){
        auto permissoesJson = j["permissoes_texto"];
        for (auto& [key, value] : permissoesJson.items()) {
            permissõesTexto[key] = value.get<std::string>();
        }
    }
}

//Obtém o texto amigável para uma permissão específica
std::string Strings::obterPermissãoTexto(std::string perm){
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
    if(permissõesTexto.find(perm) != permissõesTexto.end()){
        return permissõesTexto[perm];
    }else{
        if(defaultPermissoes.find(perm) != defaultPermissoes.end()){
            return defaultPermissoes[perm];
        }else{
            return perm;
        }
    }
}

std::vector<std::string> Strings::obterString(nlohmann::json* stringsJson, std::string sttr){
    if(*stringsJson!=json() && stringsJson->find(sttr) != stringsJson->end()){
        return stringsJson->at(sttr).get<std::vector<std::string>>();
    }else{
        return obterStringEn(sttr);
    }
    return std::vector<std::string>();
}

//Caso o JSON de idioma não seja carregado corretamente, retorna o texto em inglês embutido para a string solicitada
std::vector<std::string> Strings::obterStringEn(std::string sttr){
    if(sttr == "CARREGANDO_REPOSITORIOS"){
        return { "Loading repository list" };
    }else if(sttr == "ERRO_REPOS_N_ENCONTRADOS"){
        return { "Error listing repositories, no repositories found." };
    }else if(sttr == "SIM"){
        return { "Yes" };
    }else if(sttr == "NAO"){    
        return { "No" };
    }else if(sttr == "USANDO_SSL"){
        return { "Using SSL" };
    }else if(sttr == "ERRO_OBTER_CERTIFICADO"){
        return { "Could not obtain the certificate." };
    }else if(sttr == "CERT_NAO_CONFIAVEL"){
        return { "The certificate is not trustworthy" };
    }else if(sttr == "ERRO_HASHS_DESCONHECIDOS"){
        return { "No hashes provided by the server match the repository, remove with \"","\", or try updating the repository with \",\" assuming the repository is valid." };
    }else if(sttr == "VERIFICANDO_REPOSITORIO"){
        return { "Verifying repository " };
    }else if(sttr == "PRONTO"){
        return { "Done." };
    }else if(sttr == "ERRO_COM_INVALIDO"){
        return { "The command \"", "\" is not a valid command.\n\n" };
    }else if(sttr == "AJUDA"){
        return { "Usage: $$BIN [COMMAND]\n\nCommands:\n\n$$BIN update\n    Updates the repositories, addons and apps to the latest available online version\n\n$$BIN --add-repo <url>\n    Adds a repository, beware this command does not guarantee that the repository on the url is secure.\n    example: $$BIN --add-repo https://raw.githubusercontent.com/Eryck2486/apkm/refs/heads/main/exemplo_repositorio_root/\n\n$$BIN --rm-repo <repository-name>\n    Removes the repository\n\n$$BIN --list repos\n    Lists available repositories\n\n$$BIN --list addons\n    Lists available addons\n\n$$BIN search <term>\n    Searches for terms in all repositories\n\n$$BIN install <package>\n    Looks for the latest version of the app in the repositories and installs it.\n    example: $$BIN install com.termux\n\n$$BIN uninstall <package>\n    Uninstalls the package from the device\n\nAPKM is an open source tool, the official repository is available at https://github.com/Eryck2486/apkm/\n" };
    }else if(sttr == "REPOSITORIO_INVALIDO"){
        return { "Error: The provided repository address is not a valid repository or does not contain a valid \"index.json\" file." };
    }else if(sttr == "ADICIONANDO_REPOSITÒRIOS"){
        return { "Are you sure you want to add the repository \"","\" to your list of repositories?\nSecurity is not guaranteed in the addition process, confirm with Enter or cancel with Ctrl+C." };
    }else if(sttr == "REPOSITORIO_ADICIONADO"){
        return { "Repository added successfully!." };
    }else if(sttr == "REPOSITORIO_REMOVIDO"){
        return { "Repository \"","\" removed successfully!." };
    }else if(sttr == "REPOSITORIO_N_ENCONTRADO"){
        return { "The repository \"","\" was not found." };
    }else if(sttr == "NENHUM_REPO_ENCONTRADO"){
        return { "No repository found." };
    }else if(sttr == "MOSTR_RESULTS"){
        return { "The following results were found for: " };
    }else if(sttr == "BAIXANDO"){
        return { "Downloading package " };
    }else if(sttr == "INSTALANDO"){
        return { "Installing package " };
    }else if(sttr == "INSTALADO"){
        return { "App \"","\" installed successfully!." };
    }else if(sttr == "ERRO_INSTALAR"){
        return { "Error installing app \"","\"." };
    }else if(sttr == "ERRO_BAIXAR"){
        return { "Error downloading app \"","\"." };
    }else if(sttr == "PACOTE_N_ENCONTRADO"){
        return { "The package \"","\" was not found in any available repository." };
    }else if(sttr == "DESINSTALADO"){
        return { "App \"","\" uninstalled successfully!." };
    }else if(sttr == "ERRO_DESINSTALAR"){
        return { "Error uninstalling app \"","\"." };
    }else if(sttr == "DESINSTALANDO"){
        return { "Uninstalling package..." };
    }else if(sttr == "PERMISS_REQUERIDAS"){
        return { "Required Permissions:" };
    }else if(sttr == "QUEST_INSTALAR_APP"){
        return { "You are about to install the app \"","\""," proceed with the installation? (y/n): " };
    }else if(sttr == "INSTAL_CANCELADA"){
        return { "Installation of app \"","\" canceled by user." };
    }else if(sttr == "ADDON_INFOS"){
        return { "Displaying AddOn information: ", "Name: ","Vendor: ", "Dynamic: ","Description: ", "version: ", "Updated: " };
    }else if(sttr == "DADOS_PACOTE"){
        return { "ID: ","Name: ", "Description: ", "Version: ", "Origin: " };
    }else if(sttr == "PAC_INCOMPATIVEL"){
        return { "[INCOMPATIBLE: ","]" };
    }else if(sttr == "ERR_NO_REPOS"){
        return { "No repository found." };
    }else if(sttr == "QUEST_ADDONS_UPGRADE"){
        return { "There are updates available for the following AddOns: ","Do you want to update now? (y/n): " };
    }else if(sttr == "ADDON_ATUALIZADO"){
        return { "AddOn \"","\" updated successfully!" };
    }else if(sttr == "ADDON_N_ATUALIZADO"){
        return { "Error updating AddOn \"","\"." };
    }else if(sttr == "CHECKING_FOR_ADDON_UPGRADES"){
        return { "Checking for AddOn updates..." };
    }else if(sttr == "CHECKING_FOR_APP_UPGRADES"){
        return { "Checking for app updates..." };
    }else if(sttr == "QUEST_PACOTES_UPGRADE"){
        return { "The following apps have updates available: ","Do you want to update now? (y/n): " };
    }else if(sttr == "PACOTES_ATUALIZADOS"){
        return { "The following apps were updated: " };
    }else if(sttr == "PACOTES_N_ATUALIZADOS"){
        return { "Error updating the following apps: " };
    }else if(sttr == "SEM_ATUALIZACOES"){
        return { "All packages are up to date." };
    }
    return { "" };
}