#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "apkm.hpp"
#include "repository_manager.hpp"
#include <nlohmann/json.hpp>
#include "apkm_packages_manager.hpp"
#include "gerenciador_pacotes.hpp"
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace std;
using namespace Utilitarios;
using json = nlohmann::json;

PackageInfo::PackageInfo(std::string package, std::string appName, string versionName, long versionCode){
    this->package=package;
    this->versionName=versionName;
    this->versionCode=versionCode;
    this->appName=appName;
}

PackageInfo::~PackageInfo(){

}

bool PackageInfo::versionIsNewerThan(PackageInfo* other){
    return this->versionCode > other->getVersionCode();
}

bool PackageInfo::versionIsNewerThan(long other){
    return this->versionCode > other;
}

//Retorna o versionName do pacote, que é a versão legível para humanos, como "1.0.0" ou "2.5-beta", ao contrário do versionCode que é um número inteiro utilizado para controle de versões.
string PackageInfo::getVersionName(){
    return this->versionName;
}

//Retorna o versionCode do pacote como string, mesmo que seja um número, para facilitar a manipulação e exibição das informações.
long PackageInfo::getVersionCode(){
    return this->versionCode;
}

//Retorna o nome do aplicativo associado ao pacote, ou seja, o nome que aparece para o usuário.
string PackageInfo::getAppName(){
    return this->appName;
}

string PackageInfo::getPackage(){
    return this->package;
}


Helper::Helper(Config* config){
    this->config=config;
    string tmpJarPath = obterPastaTemporaria()+"/helper.jar";
}

Helper::~Helper(){

}

//Solicita as informações dos pacotes instalados no dispositivo, recebe um JSON com a estrutura {"packages":{"com.pacote.exemplo":{"appName":"Nome do App","vName":"Versão","vCode":123}}} e preenche o vetor de pacotes com as informações obtidas.
string Helper::getPackagesInfos(vector<PackageInfo*> pacotes){
    string jsonResposta = requisiçãoViaSocket(SOCKETNAME,"getPackagesVersions", true);
    vector<string> dadosPacotes = stringSplit(&jsonResposta, '\n');
    for(string pacoteJson : dadosPacotes){
        try
        {
            //{"package":"com.karaoke.play","appName":"KARAOKE PLAY","vCode":2,"vName":"3.0"}
            json j = json::parse(pacoteJson);
            string package = j["package"];
            string appName = j["appName"];
            long versionCode = j["vCode"];
            string versionName = j["vName"];
            PackageInfo* pacoteInfo = new PackageInfo(package, appName, versionName, versionCode);
            pacotes.push_back(pacoteInfo);
        }
        catch(exception e)
        {
            std::cerr << "Erro ao processar pacote: " << pacoteJson << std::endl;
            std::cerr << "Exceção: " << e.what() << std::endl;
        }
    }
    return jsonResposta;
}