#pragma once
#include <string>
#include "apkm.hpp"
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct Config; 
struct Strings {
    std::unordered_map<std::string, std::string> permissõesTexto;
    std::vector<std::string>
    //Definindo variáveis para suporte a diferentes idiomas:
    CARREGANDO_REPOSITORIOS,
    ERRO_REPOS_N_ENCONTRADOS,
    SIM,
    NAO,
    USANDO_SSL,
    ERRO_OBTER_CERTIFICADO,
    CERT_NAO_CONFIAVEL,
    ERRO_HASHS_DESCONHECIDOS,
    VERIFICANDO_REPOSITORIO,
    PRONTO,
    ERRO_COM_INVALIDO,
    AJUDA,
    REPOSITORIO_INVALIDO,
    ADICIONANDO_REPOSITÒRIOS,
    REPOSITORIO_ADICIONADO,
    REPOSITORIO_REMOVIDO,
    REPOSITORIO_N_ENCONTRADO,
    NENHUM_REPO_ENCONTRADO,
    MOSTR_RESULTS,
    BAIXANDO,
    INSTALANDO,
    INSTALADO,
    ERRO_INSTALAR,
    ERRO_BAIXAR,
    PACOTE_N_ENCONTRADO,
    DESINSTALADO,
    ERRO_DESINSTALAR,
    DESINSTALANDO,
    QUEST_INSTALAR_APP,
    PERMISS_REQUERIDAS,
    INSTAL_CANCELADA,
    ADDON_INFOS,
    DADOS_PACOTE,
    PAC_INCOMPATIVEL,
    ERR_NO_REPOS,
    QUEST_ADDONS_UPGRADE,
    ADDON_ATUALIZADO,
    ADDON_N_ATUALIZADO,
    CHECKING_FOR_ADDON_UPGRADES,
    CHECKING_FOR_APP_UPGRADES,
    QUEST_PACOTES_UPGRADE,
    PACOTES_ATUALIZADOS,
    PACOTES_N_ATUALIZADOS;

    std::vector<std::string> obterString(nlohmann::json* stringsJson, std::string sttr);
    std::vector<std::string> obterStringEn(std::string sttr);

    Strings(Config* config);
    std::string obterPermissãoTexto(std::string perm);
};