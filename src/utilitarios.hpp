#pragma once
#include "idiomas.hpp"
#include <string>
#include <vector>
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>

struct Strings;
// Estrutura de configuração do repositório
struct RepoConfig {
    std::string filepath;
    bool toAdd = false;
    std::string errMsg = "None";
    bool valido=false;
    bool check_expiry=false;
    std::string name;
    std::string url;
    std::vector<std::string> pinned_hashes;
    RepoConfig(bool toAdd, std::string url);
    RepoConfig();
    static RepoConfig* from_json(std::string jsonstring);
    static std::string to_json(RepoConfig& r);
    std::string to_json();
};

struct Config
{
    //Instância contendo strings do idioma local para utilização no restante da aplicação
    Strings *stringsidioma;
    //Instância global do CURL
    CURL* curl;
    int instrução;
    /*
    1 = atualizar (update)
    2 = adicionar repositório (--add-repository)
    3 = remover repositórios inválidos (remove-uknown)
    4 = instrução inválida
    */
    std::string* url;
    bool ssl = true;
    std::vector<std::string> pacotes;
    std::string nomebinario;
    Config(int argc, char* argv[]);

    static void printcfg(Config* config, Strings* stringsidioma);

    //Converte true para sim e false para não
    static std::string boolToHLang(bool opt, Strings* stringsidioma);
    std::string comandoInvalido;
};

struct RemoteRepoConfig
{
    //construtor que recebe url e retorna os dados do repositório
    static RemoteRepoConfig* fromJson(std::string jsonstring);

    //nome do repositório (opcional, sem caracteres especiais ou espaços ex: meu-repositorio)
    std::string name;
    //local onde o apkm procurará pelos apps
    std::string repository_sources_path;
    //
    std::vector<std::vector<std::string>> packages;                      
    /*Ordem exemplo
    "packages": [
        ["com.app.exemplo","Nome do APP","Descrição do app, ex: Este app abre o repositório do apkm no github", "com.app/exemplo.apk"]
    ]
    */
};

//Estrutura que contem os objetos a serem utilizados na função verify_callback
struct Tools
{
    RepoConfig* repoconfig;
    Config* configs;
    Tools(RepoConfig* repoonfig, Config* configs);
    std::string serverResponse;
};

class Utilitarios
{
    public:
        static std::string propertyReader(std::string prop);
        static void stringReplace(std::string* string, std::string alvo, std::string novotexto);
};
