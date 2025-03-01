#include "ConfigParser.hpp"
#include "../includes/ServerManager.hpp"
#include "Exception.hpp"




int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    ConfigParser configParser;
    try
    {

        if (configParser.validateConfigPath(std::string(argv[1])))
            throw Exception("Errore nel path del file di configurazione");
        std::cout << "Path corretto" << std::endl;

        TreeNode *root = configParser.parseConfigFile(std::string(argv[1]));
        if (root == NULL)
            throw Exception("Errore nel parsing del file di configurazione");
        std::cout << "Parsing completato" << std::endl;

        std::vector<TreeNode *> children = root->getChildren();
        std::vector<TreeNode *>::iterator currentNode;
        ServerManager *serverManager = new ServerManager();
        for (currentNode = children.begin(); currentNode != children.end(); ++currentNode)
        {
            if ((*currentNode)->getDirective() == "server")
            {

                Server *server = new Server();

                configParser.extractDirectives(server, *currentNode);

                if (configParser.checkMandatoryDirectives(server))
                    throw Exception("Error: missing mandatory directives");
                setUpDefaultValues(server);
                if (configParser.verifyDirectives(server))
                    throw Exception("Error: invalid directive value");
                serverManager->addServer(server);
            }
        }
    }
    catch (Exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}