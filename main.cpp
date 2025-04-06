#include "ServerManager.hpp"
#include "ConfigParser.hpp"
#include "Exception.hpp"
#include "Booter.hpp"
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

    Logger::info("Starting servers...");
    for(size_t i = 0; i < configParser.getTmpServer().size(); i++){
        Server *server = configParser.getTmpServer()[i];

        std::string host = server->getServerDir()["host"][0];
        std::string port = server->getServerDir()["listen"][0];
        std::string msg = "Server started on " + host + ":" + port;
        Logger::info(msg);
        
        Booter::bootServer(server, host, port);
        serverManager.addServer(server);
    }
    Logger::info("Server/s started successfully");

    serverManager.eventLoop();

    return 0;
}
