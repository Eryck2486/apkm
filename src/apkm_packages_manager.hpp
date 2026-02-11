#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "apkm.hpp"
#include "repository_manager.hpp"
#include <nlohmann/json.hpp>

#ifndef PACKAGE_INFO_CPP
#define PACKAGE_INFO_CPP
class PackageInfo
{
    public:
        PackageInfo(std::string appName, std::string packageName, std::string versionName, std::string versionCode);
        ~PackageInfo();
        std::string getPackageName();
        std::string getVersionName();
        std::string getVersionCode();
        std::string getAppName();
    private:
        std::string appName;
        std::string packageName;
        std::string versionName;
        std::string versionCode;
};
#endif

#ifndef PACKAGES_XML_CPP
#define PACKAGES_XML_CPP
//Classe que se comunica com o serviço HelperService
class Helper
{
    public:
        Helper(Config* config);
        ~Helper();
        std::string getPackagesInfos(std::vector<PackageInfo*> pacotes);
    private:
        std::string SOCKETNAME = "apkm_service_socket";
        Config* config;
};
#endif