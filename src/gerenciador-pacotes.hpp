#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "utilitarios.hpp"
#include "repository-manager.hpp"
class GerenciadorPacotes
{
public:
    GerenciadorPacotes(Config* config, Repomanager* repomanager) : configs(config), repomanager(repomanager) {}
    Config* configs;
    Repomanager* repomanager;
    bool pesquisar();
    bool instalarPacotes();
    bool desinstalarPacotes();
    static bool VerificarIntegridadePacote(std::string arquivoPath, std::string expectedMd5);
    static std::string calcularHashArquivo(const std::string& caminho, const EVP_MD* algoritmo);
    bool verificarApk(std::filesystem::path apkPath, DadosPacote* pacote);
private:
};