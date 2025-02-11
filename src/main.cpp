#include "ServerManager.hpp"
#include "Booter.hpp"

int main(int argc, char **argv, char**envp)
{
    (void)envp;
    ServerManager serverManager;

    if(argc == 2){
        if(!handle_arguments(argv)) return (1);
        
        // TODO: validate if config file is exist, if not dir, if is readable
        // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/19
        // TODO: finish parsing of config file
        // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/18
        //if(validateConfigPath() && parseConfigFile()) return 1;
        
        
        //----TMP------
        //al momento sto bootando un server in localhost, 
        //in futuro verra fatto nel parsing del configFile
        //(non suona bene come idea ma YOLO)
        Server *server = new Server();
        Booter booter;
        booter.bootServer(server, "localhost", "8080");
        serverManager.addServer(server);
        //-------------
        
        serverManager.mainLoop();
        return 0;
    }
    if(argc == 3){
        if(std::string(argv[1]) != "-t"){
            std::cout << "webserver [OPTIONS]: wrong number of arguments" << std::endl;
            return 1;
        }
        // if(validateConfigPath() && parseConfigFile()) return 1;
        
        return 0;
    }

    std::cout << "webserver: try 'webserver --help' for more information" << std::endl;
        return 1;
}


        