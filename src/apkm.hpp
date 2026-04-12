#pragma once
#include "idiomas.hpp"
#include <string>
#include <vector>
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include "apkm_packages_manager.hpp"

//Cores para o console
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

//Reset deve ser adicionado ao final de uma marcação de cor
#define RESET   "\033[0m"

//Definindo indicadores coloridos
#define CNLIND(msg) cout << BOLDYELLOW << "|> " << RESET << msg << endl
#define CNLINDERR(msg) cerr << BOLDRED << "|[!]> " << RESET << msg << endl
#define CNLINDINFO(msg) cout << BOLDCYAN << "|[i]> " << RESET << msg << endl
#define CNLINDINPUT(msg) cout << BOLDCYAN << "|[?]_> " << RESET << msg

//Definido indicadores sem cores
#define NCNLIND(msg) cout << "|> " << msg << endl
#define NCNLINDERR(msg) cerr << "|[!]> " << msg << endl
#define NCNLINDINFO(msg) cout << "|[i]> " << msg << endl
#define NCNLINDINPUT(msg) cout << "|[?]_> " << msg

#define LINHASEPARADORACHAR '-'


struct Strings;
// Estrutura de configuração do repositório
struct RepoConfig {
    std::string filepath; //Armazena o local onde o JSON foi lido para gerar a estrutura, util para o gerenciamento de arquivos de repositório
    bool toAdd = false; //Indica se o repositório está sendo adicionado
    std::string errMsg = "None"; //Mensagem de erro gerada durante a validação do repositório
    bool valido=false; //Indica se o repositório foi validado com sucesso
    bool check_expiry=false; //Indica se o repositório deve verificar validade do certificado SSL
    std::string name; //Nome do repositório
    std::string url; //URL do repositório
    std::vector<std::string> pinned_hashes; //Hashes permitidos para o certificado SSL do repositório
    RepoConfig(bool toAdd, std::string url); //Construtor básico para adição de um novo repositório
    RepoConfig(); //Construtor padrão
    static RepoConfig* from_json(std::string jsonstring); //Converte string JSON para estrutura RepoConfig
    static std::string to_json(RepoConfig& r); //Converte os dados da estrutura RepoConfig para string JSON (Para salvar)
    std::string to_json(); //Converte os dados da estrutura RepoConfig para string JSON sem argumentos (Refereência ao próprio objeto)
};

//Estrutura organizadora para dados de pacotes do repositório
struct DadosPacote
{
    std::string origem;
    std::string pacote; //Nome do pacote (ex: com.exemplo.app)
    std::string nome; //Nome amigável do aplicativo
    std::string descrição; //Descrição do pacote
    std::string versão; //Versão do pacote
    long versionCode; //Version code do pacote para controle de versões (Número inteiro, quanto maior mais recente)
    std::string sha256sum; //Hash SHA256 do arquivo APK
    std::string endereço; //Endereço completo para download do APK
    std::vector<std::string> arquiteturas; //Arquiteturaa suportadaa pelo pacote (ex: arm64-v8a, armv7a...)
    std::string icon; //Ícone do aplicativo em base64 (opcional, pode ser utilizado para exibir o ícone na interface do usuário)
    DadosPacote(std::string raizrepo, std::vector<std::string> dados); //Construtor que recebe vetor de strings com os dados do pacote
    DadosPacote();
    //Converte string JSON para estrutura DadosPacote
    static DadosPacote* fromJson(std::string jsonstr);
    std::string toJson();
};

//Estrutura que representa o index.json retornado pelo servidor
struct RemoteRepoConfig
{
    //Lista de hashes para validação SSL
    std::vector<std::string> pinned_hashes;
    //construtor que recebe url e retorna os dados do repositório
    static RemoteRepoConfig* fromJson(std::string jsonstring);
    static RemoteRepoConfig* fromJsonOfAddOn(std::string jsonstring, std::string addonpacote);

    std::string origem;
    //nome do repositório (opcional, sem caracteres especiais ou espaços ex: meu-repositorio)
    std::string name;
    //local onde o apkm procurará pelos apps
    std::string repository_sources_path;
    //lista de pacotes disponíveis no repositório
    std::vector<DadosPacote*> pacotes;                 
};

//Estrutura de configurações de um AddOn, funciona como um registro de todas as informações importantes do AddOn
struct AddOnConfig
{
    bool dinamico; //Indica se o AddOn é dinâmico
    std::string descrição; //Descrição do AddOn
    std::string fornecedor; //Fornecedor do AddOn
    std::string nome; //Nome de exibição
    std::string addonpacote; //Pacote do Addon no sistema
    std::string versão; //versão atual do AddOn
    bool novaversao; //Indica se há uma nova versão do AddOn disponível (Utilizado para AddOns dinâmicos que implementam self-update)
    std::string prefix; //prefixo pelo qual o usuário chamará o AddOns de forma direta, o prefixo é utilizado em seleção direta onde por exemplo no comando "apkm install fdroid:com.termux" o prefixo é fdroid
    std::string socketName; //nome atual do socket de comunicação com o AddOn que é gerado em formato de UUID aleatório para evitar conflitos entre AddOns, o nome do socket é utilizado para comunicação direta com o AddOn sem depender do binder do Android (Mais rápido e eficiente)
};

struct Config;

//Estrutura que representa e gerencia a conexão com um AddOn
//Addons podem ser estáticos (apenas retornam o RemoteRepoConfig) ou dinâmicos (bibliotecas .so que implementam funções específicas)
//Os addosn dinâmicos devem implementar as funções Buscar, ObterApkUrl e ObterExtras (Para função de UI)
//A função getRepo retorna o RemoteRepoConfig gerado pelo AddOn (Caso estático)
//Addons podem por exemplo ler um site e retornar resultados de pesquisa ou baixar APKs de fontes específicas
class AddOn {
public:
    //Instanciador padrão
    AddOn(std::string addonpacote, Config* mainCfg);
    //Destrutor padrão
    ~AddOn();
    //Função que efetua a busca por um termo e retorna lista de pacotes encontrados no AddOn
    std::vector<DadosPacote*> Buscar(std::string pesquisa);
    //Carrega todos os AddOns disponíveis no sistema
    static std::vector<AddOn*> CarregarTodos(Config* config);
    //Obtem os repositórios fornecidos pelo AddOn estático
    std::vector<RemoteRepoConfig*> getRepos();
    //Solicita um APK pelo pacote ao AddOn
    std::vector<std::string> getPackage(std::string pacotestr);
    //Solicita o apk de atualização do AddOn (self-update)
    std::vector<std::string> getAddonUpdate();
    //Recebe uma lista dos pacotes instalados e retorna um JSON com os Apps que podem ser atualizados
    std::vector<PackageInfo*> getPackagesUpdatesFromJSON(std::string jsonstr);
    //Obtem a configuração do AddOn
    bool getConfig();
    //Função de query principal, essa é a "ponte" que permite que o APKM se comunique com os APKs que são AddOns
    std::vector<std::string> ContentQuery(std::string requisição);
    //Mostra as informações do AddOn no terminal
    void exibirAddOnInfos(Config* conf);
    //Função de comunicação via socket para o AddOn (Mais rápido pois não depende do binder do Android diretamente)
    std::string Call(std::string request, bool aguardarResposta);
    //Função para inicializar o serviço do AddOn caso esteja parado
    bool startAddOn();
    //Configurações do AddOn 
    AddOnConfig* config;
    //Configurações globais
    Config* mainCfg;
    //Comunicação de dowload de arquivos no AddOn (Utilizada para obter o status do download e exibir barra de progresso)
    std::vector<std::string> initDownloadComunication(std::string request);
private:
    //Cria um socket para que o AddOn envie o estado de tarefa atual como por exemplo % do download
    bool iniciarSocketFeedback(bool &key);
    static void barraProgresso(int porcentagem, std::string texto);
};

struct Config
{
    //Nome do socket de feedback do APKM
    std::string socketName = "apkm_socket_feedback";
    //Variável que define se o terminal será colorido
    bool terminalColor = true;
    //Variável definida por -y ou --yes para assumir "sim" em todas as perguntas
    bool assumirSim = false;
    //Determina se pacotes incompatíveis serão exibidos nas pesquisar
    bool exibirIncompatíveis = false;
    //Define se a saída para o teminal será o formato JSON bruto ou interface amigável para humanos
    bool formatoJSON = false;
    //lista de AddOns dinâmicos não estáticos
    std::vector<AddOn*> addonsdinamicos;
    //pacote padrão para AddOns
    std::string pacoteAddOns = "com.apkm.addon.";
    //lista de todos os repositórios
    std::vector<RemoteRepoConfig*> reposglobais;
    //local onde o apkm trabalha
    std::string diretórioDados = "/data/apkm"; //Pasta de dados do apkm
    std::string diretórioModuloSources = "/system/etc/apkm"; //Pasta de repositórios do módulo Magisk (somente leitura, é um magisk-mount dos arquivos do módulo)
    std::string diretórioSources = diretórioDados+"/sources"; //Pasta de repositórios
    std::string diretórioIdiomas = diretórioModuloSources+"/locales"; //Pasta de idiomas
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
    6 = pesquisar pacote (search)
    7 = instalar pacote (install)
    8 = desinstalar pacote (uninstall)
    */
    std::string url; //URL do repositório (utilizado em instrução 2)
    bool ssl = true; //Indica se a verificação SSL está habilitada (padrão: true)
    //Lista de nomes de pacotes (utilizado em instruções 6, 7 e 8)
    std::vector<std::string> nomes;
    //Nome do binário utilizado para invocar o programa
    std::string nomebinario;
    //Construtor que prepara as configurações da execução para o processamento da ordem
    Config(int argc, char* argv[]);
    bool argParam(std::string argstr);
    //Destrutor que limpa a instância
    ~Config();

    //Mostra as configurações da execução
    static void printcfg(Config* config, Strings* stringsidioma);

    //Converte true para sim e false para não
    static std::string boolToHLang(bool opt, Strings* stringsidioma);
    //Armazena comando inválido caso seja detectado
    std::string comandoInvalido;
};

//Estrutura que contem os objetos a serem utilizados na função verify_callback (verificação de URL SSL)
struct Tools
{
    bool linkVálido = false; //Indica se o link do repositório foi acessado com sucesso (Passou na verificação de certificado)
    bool usarSSL = false; //Estado do SSL
    std::string url; //Armazena a URL do repositório/site sendo validado
    std::string toolname; //Nome do objeto que está utilizando a struct (Para mensagens de erro)
    std::vector<std::string> certificadoHashs; //Armazena os hashes do certificado SSL obtidos durante a conexão
    Config* configs; //Ponteiro para configuração geral do programa
    Tools(std::vector<std::string> certificadoHashs, std::string url, Config* configs, bool usarSSL); //Construtor para struct tools
    std::string errorMsg; //Armazena mensagem de erro gerada durante a validação
    std::string serverResponse; //Armazena a resposta do servidor (JSON do repositório)
};

//Coleção de funções utilitárias gerais
namespace Utilitarios
{
    std::string getProp(std::string prop);
    void stringReplace(std::string* string, std::string alvo, std::string novotexto);
    std::vector<std::string> stringSplit(std::string* str, char alvo);
    bool stringContains(std::string* str, std::string alvo);
    bool search_match(std::string fonte, std::string termo);
    std::string obterPastaTemporaria();
    std::string executarComandoShell(std::string cmd);
    std::string limparSaidaContent(std::string bruto);
    bool checarCompatibilidade(std::vector<std::string> abisPacote);
    int obterLarguraTerminal();
    std::string gerarLinhaSeparadora();
    void NLIND(std::string msg);
    void NLINDERR(std::string msg);
    void NLINDINFO(std::string msg);
    void NLINDINFO(std::string msg);
    void NLINDINPUT(std::string msg);
    bool terminalColor();
    std::string requisiçãoViaSocket(std::string SOCKETNAME, std::string function, bool aguardarResposta);
    //Imnprime uma menssagewm de erro no terminal
    void printInfo(std::string tipo, std::string msg);
};

//Representação mínima do manifesto Android (AndroidManifest.xml) de um APK para a validação
struct APKManifesto
{
    std::string packageName; //Nome do pacote
    std::string versionName; //Versão do pacote
    int versionCode; //Código da versão
    int minSdkVersion; //Versão mínima do SDK
    int targetSdkVersion; //Versão alvo do SDK
    std::vector<std::string> permissions; //Lista de permissões solicitadas
    bool manifestoValido = false; //Indica se o manifesto foi lido com sucesso
};