#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "apkm.hpp"
#include "repository_manager.hpp"
#include <nlohmann/json.hpp>

// Estrutura simplificada para ler o cabeçalho de um chunk AXML
struct AxmlChunkHeader {
    uint16_t type;
    uint16_t headerSize;
    uint32_t size;
};

//Gerenciador de Pacotes
class GerenciadorPacotes
{
public:
    GerenciadorPacotes(Config* config, Repomanager* repomanager) : configs(config), repomanager(repomanager) {}
    void pesquisar();
    bool prepararInstalarPacotes();
    bool desinstalarPacotes();
private:
    Config* configs;
    Repomanager* repomanager;
    static bool VerificarIntegridadePacote(std::string arquivoPath, std::string expectedMd5);
    static std::string calcularHashArquivo(const std::string& caminho, const EVP_MD* algoritmo);
    static std::vector<std::string> extrairStringsAxml(const std::vector<uint8_t>& buffer);
    APKManifesto* verificarApk(std::filesystem::path apkPath, DadosPacote* pacote);
    static void mostrarListaResultados(std::vector<DadosPacote*> pacotes, Config* configs);
    bool instalarPacote(DadosPacote* pacote, RemoteRepoConfig* repoConfig);
};