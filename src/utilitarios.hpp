#pragma once
#include "idiomas.hpp"
#include <string>
#include <vector>
#include <curl/curl.h>
#include <iostream>

struct Strings;
// Estrutura de configuração do repositório
struct RepoConfig {
    bool toAdd = false;
    std::string errMsg;
    bool valido=false;
    bool check_expiry=false;
    std::string name;
    std::string url;
    std::vector<std::string> pinned_hashes;
    RepoConfig(bool toAdd, std::string url);
    RepoConfig();
};

struct Config
{
    //Instância contendo strings do idioma local para utilização no restante da aplicação
    Strings *stringsidioma;
    //Instância global do CURL
    CURL* curl;
    int instrução;
    std::string* url;
    /*
    0 = atualizar (update)
    1 = adicionar repositório (--add-repository)
    2 = remover repositórios inválidos (remove-uknown)
    */
    bool ssl = true;
    std::vector<std::string> pacotes;
    std::string nomebinario;
    Config(int argc, char* argv[]);

    static void printcfg(Config* config, Strings* stringsidioma);

    //Converte true para sim e false para não
    static std::string boolToHLang(bool opt, Strings* stringsidioma);
};

struct RemoteRepoConfig
{
    //nome do repositório (opcional, sem caracteres especiais ou espaços ex: meu-repositorio)
    std::string name;
    //local onde o apkm procurará pelos apps
    std::string repository_sources_path;
    //
    std::vector<std::vector<std::string>> packages;
    /*Ordem exemplo
    "packages": [
        ["com.app.exemplo","Nome do APP","Descrição do app, ex: Este app abre o repositório do apkm no github"]
    ]
    
    O nome do pacote deve ser exemplo.apk e deve estar na pasta com.app que por sua vez está na repository_sources_path exemplo:
    /home/user/com.app/exemplo.apk
    caso queira pode incluir: exemplo.icon.png para interface gráfica (opcional)
    */
};

class Utilitarios
{
    public:
        static std::string propertyReader(std::string prop);
};
