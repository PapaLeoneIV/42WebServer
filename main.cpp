#include "ServerManager.hpp"
#include "Booter.hpp"
#include "ConfigParser.hpp"
#include "Exception.hpp"
#include "Logger.hpp"
int main(int argc, char **argv)
{
    if (!handle_arguments(argc, argv))
        return (1);

    ServerManager serverManager;
    ConfigParser configParser;
 
    if (configParser.validatePath(std::string(argv[1])) || configParser.fromConfigFileToServers(argv[1])){
        Logger::error(argv[1], "invalid configuration file");
        return 1;
    }

    Booter booter;
    for(size_t i = 0; i < configParser.getTmpServer().size(); i++){
        Server *server = configParser.getTmpServer()[i];
        booter.bootServer(server,server->getServerDir()["host"][0].c_str(), server->getServerDir()["listen"][0].c_str());
        Logger::info("Server started on " + server->getServerDir()["host"][0] + ":" + server->getServerDir()["listen"][0]);
        serverManager.addServer(server);
    }
    
    serverManager.eventLoop();

    return 0;
}
