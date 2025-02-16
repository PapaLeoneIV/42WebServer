#ifndef  CONFIGPARSER_HPP 
#define CONFIGPARSER_HPP

#include <exception>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstddef>
#include <stdlib.h>

#include <limits.h>

#include <string>
#include <vector>
#include <map>
#include <stack>

#include "../includes/Server.hpp"


typedef int (*FunctionPtr)(std::vector<std::string>);



    
class TreeNode{
    public:
        TreeNode(std::string directive, std::vector<std::string> value){
            this->directive = directive;
            this->value = value;
        }
        void add(TreeNode* node){
            children.push_back(node);
        }
        void print(int level = 0) {
        for (int i = 0; i < level; ++i)
            std::cout << "  ";
        std::cout << directive;
        if (!value.empty()){
            size_t i = 0;
            while(i < value.size()){
                std::cout << " " << value[i];
                i++;
            }
        }
        std::cout << std::endl;
        for (size_t i = 0; i < children.size(); ++i)
            children[i]->print(level + 1);
        }
        std::string &getDirective(){
            return directive;
        }
        std::vector<std::string> &getValue(){
            return value;
        }
        std::vector<TreeNode*> &getChildren(){
            return children;
        }
    private:
        std::string directive;
        std::vector<std::string> value;
        std::vector<TreeNode*> children;
};


class ConfigParser {
    public:
    int validateConfigPath(std::string path);
    TreeNode * parseConfigFile(std::string path);
    int isValidDirective(std::string token);

    

    ConfigParser();
    ~ConfigParser();

    std::map<std::string, FunctionPtr> fnToParseDirectives;
    std::vector<std::string> directives;
    std::vector<std::string> extensionsAllowd;

};



std::string removeComments(std::ifstream &file);
std::string trimLeftRight(const std::string &str); 
std::string removeEmptyLines(const std::string &input); 
std::string trimm(const std::string &input);
int checkMandatoryDirectives(Server *server);
int setUpDefaultValues(Server *server);


#endif //CONFIGPARSER