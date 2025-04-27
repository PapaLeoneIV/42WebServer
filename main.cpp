#include "./includes/ServerManager.hpp"
#include "./includes/ConfigParser.hpp"
#include "./includes/Booter.hpp"
#include "./includes/Logger.hpp"

//TODO: - upload some file to the server and get it back
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/44
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
    std::vector<std::string> hostPortKeys;
    std::vector<Server *> servers;
    for(size_t i = 0; i < configParser.getTmpServer().size(); i++){
    

        Server *server = configParser.getTmpServer()[i];
        std::string host = server->getServerDir()["host"][0];
        std::string port = server->getServerDir()["listen"][0];
        std::string msg = "Server started on " + host + ":" + port;
        server->setHostPortKey(host + ":" + port);
        Logger::info(msg);
        bool shouldBoot = true;
        for(size_t i = 0; i < hostPortKeys.size(); i++){
            if(server->getHostPortKey() == hostPortKeys[i]){
                server->setServerSocket(servers[i]->getServerSocket());
                shouldBoot = false;
            }
        }
        
        hostPortKeys.push_back(server->getHostPortKey());
        servers.push_back(server);
        if(shouldBoot)
            Booter::bootServer(server, host, port);
        serverManager.addServer(server);
    }
    Logger::info("Server/s started successfully");
    serverManager.eventLoop();

    return 0;
}





// adesso ho una mappa ma non posso gestire servers con stesso host:protected
// devo trasformare la mappa in un vettore di Server.
// Dentro ogni server devo avere una chiave host:port
// quando booto il server devo controllare se c'è un server con la stessa chiave
// se ce prendo il socket del server precedente e lo lego a quello nuovo
// Quando ricevo una richiesta devo cercare il server tramite la chiave e se ce ne sono due uguali controllare l header host
// scegliere nel caso di ambiguita il primo server nell arrray