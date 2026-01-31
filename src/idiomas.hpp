#pragma once
#include <string>
#include "utilitarios.hpp"
#include <vector>

struct Strings {
    std::vector<std::string> CARREGANDO_REPOSITORIOS;
    std::vector<std::string> ERRO_REPOS_N_ENCONTRADOS;
    std::vector<std::string> SIM;
    std::vector<std::string> NAO;
    std::vector<std::string> USANDO_SSL;
    std::vector<std::string> ERRO_OBTER_CERTIFICADO;
    std::vector<std::string> CERT_NAO_CONFIAVEL;
    std::vector<std::string> ERRO_HASHS_DESCONHECIDOS;
    std::vector<std::string> VERIFICANDO_REPOSITORIO;
    std::vector<std::string> PRONTO;
    std::vector<std::string> ERRO_COM_INVALIDO;
    std::vector<std::string> AJUDA;
    std::vector<std::string> REPOSITORIO_INVALIDO;
    std::vector<std::string> ADICIONANDO_REPOSITÒRIOS;
    std::vector<std::string> REPOSITORIO_ADICIONADO;
    std::vector<std::string> REPOSITORIO_REMOVIDO;
    std::vector<std::string> REPOSITORIO_N_ENCONTRADO;
    std::vector<std::string> NENHUM_REPO_ENCONTRADO;
    std::vector<std::string> MOSTR_RESULTS;
    std::vector<std::string> BAIXANDO;
    std::vector<std::string> INSTALANDO;
    std::vector<std::string> INSTALADO;
    std::vector<std::string> ERRO_INSTALAR;
    std::vector<std::string> ERRO_BAIXAR;
    std::vector<std::string> PACOTE_N_ENCONTRADO;

    Strings(std::vector<std::vector<std::string>> textos);
    static Strings* obterStrings();
};