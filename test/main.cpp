#include "ConfigParser.hpp"
#include "../includes/ServerManager.hpp"
#include "Exception.hpp"
#include "Logger.hpp"




int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    ConfigParser configParser;
    try{
        if (configParser.validateConfigPath(std::string(argv[1])))
            throw Exception("invalid path for configuration file");
        configParser.setFileName(argv[1]);
        TreeNode *root = configParser.parseConfigFile(std::string(argv[1]));
        if (root == NULL)
            throw Exception("invalid configuration file");
        
        std::vector<TreeNode *> children = root->getChildren();
        std::vector<TreeNode *>::iterator currentNode;
        ServerManager *serverManager = new ServerManager();
        for (currentNode = children.begin(); currentNode != children.end(); ++currentNode){
            if ((*currentNode)->getDirective() == "server")
            {
                Server *server = new Server();
                
                configParser.extractDirectives(server, *currentNode);
                
                if (configParser.checkMandatoryDirectives(server))
                    throw Exception("missing mandatory directives");
                
                setUpDefaultValues(server);
                
                if (configParser.verifyDirectives(server))
                    throw Exception("invalid directive value");
                
                serverManager->addServer(server);
            }
        }
    }
    catch (Exception &e)
    {
        Logger::error(configParser.getFileName(), e.what());
        exit(1);
    }

    std::cout << "Configurazione completata" << std::endl;
    return 0;
}