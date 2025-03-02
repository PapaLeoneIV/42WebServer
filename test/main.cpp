#include "ConfigParser.hpp"
#include "../includes/ServerManager.hpp"
#include "Exception.hpp"
#include "Logger.hpp"


int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    ConfigParser configParser;
    ServerManager serverManager;
    try{
        
        if (configParser.validatePath(std::string(argv[1])))
            throw Exception("invalid path for configuration file");
        
        configParser.fromConfigFileToServers(&serverManager, argv);
    }
    catch (Exception &e){
        Logger::error(configParser.getFileName(), e.what());
        exit(1);
    }

    std::cout << "Configurazione completata" << std::endl;
    return 0;
}



