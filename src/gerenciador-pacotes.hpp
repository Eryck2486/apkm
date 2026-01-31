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
private:
};