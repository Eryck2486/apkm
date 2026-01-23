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
    ERRO_COM_INVALIDO=textos[10];
    AJUDA=textos[11];
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
                {/*PRONTO*/"Pronto."},
                {/*ERRO_COM_INVALIDO*/"O comando \"", "\" não é um comando válido.\n\n"},
                {/*AJUDA*/\
                    "$$BIN Uso: $$BIN  [COMANDO]\n"
                    "\n"
                    "Comandos:\n"
                    "\n"
                    "$$BIN update\n"
                    "   Atualiza os repositórios e apps para a versão mais recente disponível online\n"
                    "\n"
                    "$$BIN --add-repository <url>\n"
                    "   Adiciona um repositório, cuidado este comando não garante que o repositório na url é seguro.\n"
                    "   exemplo: $$BIN --add-repository https://raw.githubusercontent.com/Eryck2486/apkm/refs/heads/main/exemplo_repositorio_root/\n"
                    "\n"
                    "$$BIN remove-uknown\n"
                    "   Remove repositórios defeituosos que não passam nos testes\n"
                    "\n"
                    "$$BIN install <pacote>\n"
                    "   procura a versão mais recente do app nos repositórios e instala.\n"
                    "   exemplo: $$BIN install com.termux\n"
                    "\n"
                    "APKM é uma ferramenta de código aberto, o repositório oficial está disponível em https://github.com/Eryck2486/apkm/\n"}
            }
        );
    }
    return idioma;
}


