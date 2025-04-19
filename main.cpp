#include "./includes/ServerManager.hpp"
#include "./includes/ConfigParser.hpp"
#include "./includes/Exception.hpp"
#include "./includes/Booter.hpp"
#include "./includes/Logger.hpp"


//TODO: - The first server for a host:port will be the default for this host:port (meaning it will respond to all requests that do not belong to another server).
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/45
//TODO: - upload some file to the server and get it back
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/44
//TODO: - be able to have server on the same port and url
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/43
//TODO: - Set up the server_names or not.
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/42
//TODO: - setup multiple servers with different hostname (use something like: curl --resolve example.com:80:127.0.0.1http://example.com/)
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/41
//TODO: - handle cgi fd from fork into select
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/40
int main(int argc, char **argv)
{
    if (handle_arguments(argc, argv))
        return (1);

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
