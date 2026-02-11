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

PackageInfo::PackageInfo(std::string appName, std::string packageName, string versionName, string versionCode){
    this->packageName=packageName;
    this->versionName=versionName;
    this->versionCode=versionCode;
    this->appName=appName;
}

PackageInfo::~PackageInfo(){

}

//Retorna o nome do pacote, como "com.exemplo.app", que é a identificação única do aplicativo no sistema Android.
string PackageInfo::getPackageName(){
    return this->packageName;
}

//Retorna o versionName do pacote, que é a versão legível para humanos, como "1.0.0" ou "2.5-beta", ao contrário do versionCode que é um número inteiro utilizado para controle de versões.
string PackageInfo::getVersionName(){
    return this->versionName;
}

//Retorna o versionCode do pacote como string, mesmo que seja um número, para facilitar a manipulação e exibição das informações.
string PackageInfo::getVersionCode(){
    return this->versionCode;
}

//Retorna o nome do aplicativo associado ao pacote, ou seja, o nome que aparece para o usuário.
string PackageInfo::getAppName(){
    return this->appName;
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
    try{
        json resposta = json::parse(jsonResposta);
        if(resposta.find("packages")!=resposta.end()){
            if(resposta["packages"].is_object()){
                for(auto it = resposta["packages"].begin(); it != resposta["packages"].end(); ++it){
                    PackageInfo* pacote = new PackageInfo(it.value()["appName"], it.key(), it.value()["vName"], to_string(it.value()["vCode"]));
                    pacotes.push_back(pacote);
                }
            }
        }
    }catch(json::exception& e){
        NLINDERR(jsonResposta);
        NLINDERR(e.what());
    }
    return jsonResposta;
}