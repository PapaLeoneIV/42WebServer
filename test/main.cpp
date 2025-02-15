#include "ConfigParser.hpp"
int main(int argc, char **argv){
    if(argc != 2) return 1;
    ConfigParser configParser;
    int res = configParser.validateConfigPath(std::string(argv[1]));
    if (res)
        std::cout << "Path o file sbagliato" << std::endl; 
    else std::cout << "Path corretto" << std::endl;
    TreeNode * res2 = configParser.parseConfigFile(std::string(argv[1]));
    if (res2 == NULL){
        std::cout << "Parsing andato male" << std::endl; 
        return 1;
    }
    else std::cout << "Parsing completato" << std::endl;

    res2->print();

}