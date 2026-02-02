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
    REPOSITORIO_INVALIDO=textos[12];
    ADICIONANDO_REPOSITÒRIOS=textos[13];
    REPOSITORIO_ADICIONADO=textos[14];
    REPOSITORIO_REMOVIDO=textos[15];
    REPOSITORIO_N_ENCONTRADO=textos[16];
    NENHUM_REPO_ENCONTRADO=textos[17];
    MOSTR_RESULTS=textos[18];
    BAIXANDO=textos[19];
    INSTALANDO=textos[20];
    INSTALADO=textos[21];
    ERRO_INSTALAR=textos[22];
    ERRO_BAIXAR=textos[23];
    PACOTE_N_ENCONTRADO=textos[24];
    DESINSTALADO=textos[25];
    ERRO_DESINSTALAR=textos[26];
    DESINSTALANDO=textos[27];
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
                {/*VERIFICANDO_REPOSITORIO*/"Verificando repositório "},
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
                    "APKM é uma ferramenta de código aberto, o repositório oficial está disponível em https://github.com/Eryck2486/apkm/\n"},
                {/*REPOSITORIO_INVALIDO*/"Erro: O endereço de repositório fornecido não é um repositório válido ou não contem um arquivo \"index.json\" válido."},
                {/*ADICIONANDO_REPOSITÒRIOS*/"Tem certeza que quer adicionar o repositório \"","\" a sua lista de repositórios?\nA segurança não é garantida no processo de adição, confirme com Enter ou cancele com Ctrl+C."},
                {/*REPOSITORIO_ADICIONADO*/"Rpositório adicionado com sucesso!."},
                {/*REPOSITORIO_REMOVIDO*/"Repositório \"","\" Removido com sucesso!"},
                {/*REPOSITORIO_N_ENCONTRADO*/"Não foi possível encontrar um repositório chamado \"", "\" Abortar!."},
                {/*NENHUM_REPO_ENCONTRADO*/"Nenhum repositório encontrado."},
                {/*MOSTR_RESULTS*/"Os seguintes pacotes foram encontrados para: "},
                {/*BAIXANDO*/"Baixando pacote "},
                {/*INSTALANDO*/"Instalando pacote "},
                {/*INSTALADO*/"App \"","\" instalado com sucesso."},
                {/*ERRO_INSTALAR*/"Erro ao instalar o app \"", "\"."},
                {/*ERRO_BAIXAR*/"Erro ao baixar o app \"", "\"."},
                {/*PACOTE_N_ENCONTRADO*/"O pacote \"", "\" não foi encontrado em nenhum repositório disponível."},
                {/*DESINSTALADO*/"App \"","\" desinstalado com sucesso."},
                {/*ERRO_DESINSTALAR*/"Erro ao desinstalar o app \"", "\"."},
                {/*DESINSTALANDO*/"Desinstalando pacote "}
            }
        );
    }
    return idioma;
}


