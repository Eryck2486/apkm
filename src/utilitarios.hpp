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

//Estrutura organizadora para dados de pacotes do repositório
struct DadosPacote
{
    std::string pacote;
    std::string nome;
    std::string descrição;
    std::string versão;
    std::string md5sum;
    std::string endereço;
    std::string arquitetura;
    DadosPacote(std::string raizrepo, std::vector<std::string> dados);
};

//Estrutura que representa o index.json retornado pelo servidor
struct RemoteRepoConfig
{
    //construtor que recebe url e retorna os dados do repositório
    static RemoteRepoConfig* fromJson(std::string jsonstring);

    std::string origem;
    //nome do repositório (opcional, sem caracteres especiais ou espaços ex: meu-repositorio)
    std::string name;
    //local onde o apkm procurará pelos apps
    std::string repository_sources_path;
    //
    std::vector<DadosPacote*> pacotes;                      
    /*Ordem exemplo
    "packages": [
        ["com.app.exemplo","Nome do APP","Descrição do app, ex: Este app abre o repositório do apkm no github", "com.app/exemplo.apk"]
    ]
    */
};


struct AddOnConfig
{
    bool dinamico;
    std::string addonpath;
    std::string novaversao;
};

struct Config;
class AddOn
{
public:
    AddOnConfig* config;
    AddOn(std::string addonpath);
    ~AddOn();
    std::string Buscar(std::string pesquisa);
    std::string ObterApkUrl(std::string pacote);
    std::string ObterExtras(std::string pacote);
    RemoteRepoConfig* getRepo();
    static std::vector<AddOn*> CarregarTodos(Config* config);
private:
    void getConfig();
};

struct Config
{
    //indicador de nova linha
    std::string nlind = "|->";
    std::string nlinderr = "|[!]->";
    //lista de AddOns dinâmicos não estáticos
    std::vector<AddOn*> addonsdinamicos;
    //lista de todos os repositórios
    std::vector<RemoteRepoConfig*> reposglobais;
    //local onde o apkm trabalha
    std::string diretórioDados = "/data/apkm";
    std::string diretórioSources = diretórioDados+"/sources";
    std::string diretórioAddOns = diretórioDados+"/addons";
    std::string diretórioTmp = "/data/local/tmp/apkm-tmp";
    //Instância contendo strings do idioma local para utilização no restante da aplicação
    Strings *stringsidioma;
    //Instância global do CURL
    CURL* curl;
    int instrução;
    /*
    1 = atualizar (update)
    2 = adicionar repositório (--add-repo)
    3 = remover repositório (--rm-repo)
    4 = --list (repos ou addons)
    5 = --list addons
    */
    std::string url;
    bool ssl = true;
    std::vector<std::string> nomes;
    std::string nomebinario;
    Config(int argc, char* argv[]);
    ~Config();

    static void printcfg(Config* config, Strings* stringsidioma);

    //Converte true para sim e false para não
    static std::string boolToHLang(bool opt, Strings* stringsidioma);
    std::string comandoInvalido;
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
    static bool search_match(std::string fonte, std::string termo);
    static std::string obterPastaTemporaria();
};