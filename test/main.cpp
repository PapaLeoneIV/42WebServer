#include "ConfigParser.hpp"
#include "../includes/ServerManager.hpp"
#include "Exception.hpp"
void traverseTree(Server *server, TreeNode *node) {
    if(node == NULL)
        return ;
    std::vector<TreeNode*> children = node->getChildren();
    std::vector<TreeNode*>::iterator currentNode;
    
    for(currentNode = children.begin(); currentNode != children.end(); ++currentNode){
        if((*currentNode)->getChildren().size() == 0){
            if(node->getDirective() == "server"){
                server->setServerDir((*currentNode)->getDirective(), (*currentNode)->getValue());
            }
            if(node->getDirective() == "location"){
                if(node->getValue().size() > 1){
                    std::cout << "TODO: Error: location directive can't have more than one value" << std::endl;
                    exit(1); 
                }
                server->setLocationDir(node->getValue()[0], (*currentNode)->getDirective(), (*currentNode)->getValue());
            }
        }
        traverseTree(server, *currentNode);
    }
}

int main(int argc, char **argv){
    if(argc != 2) return 1;
    ConfigParser configParser;
    try{
        
        if (configParser.validateConfigPath(std::string(argv[1])))
            throw Exception("Errore nel path del file di configurazione");
        else
            std::cout << "Path corretto" << std::endl;
        
        TreeNode * root = configParser.parseConfigFile(std::string(argv[1]));
        if (root == NULL)
            throw Exception("Errore nel parsing del file di configurazione");
        else 
            std::cout << "Parsing completato" << std::endl;

        std::vector<TreeNode*> children = root->getChildren();
        std::vector<TreeNode*>::iterator currentNode;
        ServerManager *serverManager = new ServerManager();
        for(currentNode = children.begin(); currentNode != children.end(); ++currentNode){
            if((*currentNode)->getDirective() == "server"){

                Server *server = new Server();

                traverseTree(server, *currentNode);

                if(checkMandatoryDirectives(server))
                    throw Exception("Error: missing mandatory directives");
                setUpDefaultValues(server);

                //parseDirectives(server);

                serverManager->addServer(server);
            }
        }
    }catch(Exception &e){
        std::cout << e.what() << std::endl;
    }
    
    // std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator locationDirIt;
    // std::map<int, Server*>::iterator serverIt;
    // for(serverIt = serverManager->getServerMap().begin(); serverIt != serverManager->getServerMap().end(); ++serverIt){
    //     std::map<std::string, std::vector<std::string>> serverDir = (*serverIt).second->getServerDir();
    //     for (std::map<std::string, std::vector<std::string>>::iterator serverDirIt = serverDir.begin(); serverDirIt != serverDir.end(); ++serverDirIt) {
            // std::string nginxDir = (*serverDirIt).first;
            // std::vector<std::string> nginxDirValue = (*serverDirIt).second;
            //  this->fnToParseDirectives[nginxDir](nginxDirValue);
            //  }
    //     std::map<std::string, std::map<std::string, std::vector<std::string>>> locationDir = (*serverIt).second->getLocationDir();
    //     for (locationDirIt = locationDir.begin(); locationDirIt != locationDir.end(); ++locationDirIt) {
    //         std::map<std::string, std::vector<std::string>> locationDirMap = (*locationDirIt).second;
    //         for (std::map<std::string, std::vector<std::string>>::iterator locationDirMapIt = locationDirMap.begin(); locationDirMapIt != locationDirMap.end(); ++locationDirMapIt) {
    //             this->fnToParseDirectives[(*locationDirMapIt).first]((*locationDirMapIt).second);
    //         }
    //     }
    // }
}