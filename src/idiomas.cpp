#include <locale>
#include <iostream>
#include <vector>
#include <sys/system_properties.h>
#include "idiomas.hpp"

using namespace std;

Strings::Strings(std::vector<std::vector<std::string>> textos){
    CARREGANDO_REPOSITORIOS=textos[0];
    ERRO_REPOS_N_ENCONTRADOS=textos[1];
    SIM=textos[2];
    NAO=textos[3];
    USANDO_SSL=textos[4];
    ERRO_OBTER_CERTIFICADO=textos[5];
    CERT_NAO_CONFIAVEL=textos[6];
    ERRO_HASHS_DESCONHECIDOS=textos[7];
    VERIFICANDO_REPOSITORIO=textos[8];
    PRONTO=textos[9];
}

Strings* Strings::obterStrings(){
    std::string idiomastr = Utilitarios::propertyReader("persist.sys.locale");
    if(idiomastr=="none") idiomastr = "en-US";
    Strings* idioma = nullptr;
    if(idiomastr=="pt-BR"){
        idioma = new Strings(
            {
                {/*CARREGANDO_REPOSITORIOS*/"Carregando lista de repositórios"},
                {/*ERRO_REPOS_N_ENCONTRADOS*/"Erro ao listar repositórios, baixando repositórios originais"},
                {/*SIM*/"Sim"},
                {/*NAO*/"Não"},
                {/*USANDO_SSL*/"Usando SSL"},
                {/*ERRO_OBTER_CERTIFICADO*/"Não foi possível obter o certificado."},
                {/*CERT_NAO_CONFIAVEL*/"O certificado não é confiável"},
                {/*ERRO_HASHS_DESCONHECIDOS*/"Nenhum hash fornecido pelo servidor coincide com o repositório, remova com ",", ou tente atualizar o repositório com "," supondo que o repositório é válido."},
                {/*VERIFICANDO_REPOSITORIO*/"Verificando repositório: "},
                {/*PRONTO*/"Pronto."}
            }
        );
    }
    return idioma;
}


