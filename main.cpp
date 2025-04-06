#include "ServerManager.hpp"
#include "Booter.hpp"
#include "ConfigParser.hpp"
#include "Exception.hpp"
#include "Logger.hpp"
int main(int argc, char **argv)
{
    if (handle_arguments(argc, argv))
        return (1);

    Logger::info("webserver: starting...");
    ServerManager serverManager;
    ConfigParser configParser;
 
    if (configParser.validatePath(std::string(argv[1])) || configParser.fromConfigFileToServers(argv[1])){
        Logger::error(argv[1], "invalid configuration file");
        return 1;
    }
    Logger::info("Configuration file loaded successfully");

    Booter booter;
    Logger::info("Starting servers...");
    for(size_t i = 0; i < configParser.getTmpServer().size(); i++){
        Server *server = configParser.getTmpServer()[i];

        std::string host = server->getServerDir()["host"][0];
        std::string port = server->getServerDir()["listen"][0];
        std::string msg = "Server started on " + std::string(host) + ":" + std::string(port);
        Logger::info(msg);
        
        if (!server->getServerDir()["index"].empty()) {
            server->setIndex(server->getServerDir()["index"][0]);
            Logger::info("Server index file configurato: " + server->getIndex());
        }
        
        booter.bootServer(server, host.c_str(), port.c_str());
        serverManager.addServer(server);
    }
    Logger::info("Servers started successfully");

    serverManager.eventLoop();

    return 0;
}
