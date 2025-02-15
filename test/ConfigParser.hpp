#ifndef  CONFIGPARSER_HPP 
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

#include <string>
#include <vector>
#include <stack>
#include <map>



typedef int (*FunctionPtr)(std::vector<std::string>);

class ConfigParser{
    public:
    int validateConfigPath(std::string path);
    int parseConfigFile(std::string path);

    int isValidDirective(std::string token);

    ConfigParser();
    ~ConfigParser();

    std::map<std::string, FunctionPtr> fnToParseDirectives;
    std::vector<std::string> directives;

};

class TreeNode{
    public:
        TreeNode(std::string directive, std::string value){
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
        if (!value.empty())
            std::cout << ": " << value;
        std::cout << std::endl;
        for (size_t i = 0; i < children.size(); ++i)
            children[i]->print(level + 1);
        }
        std::string &getDirective(){
            return directive;
        }
        std::string &getValue(){
            return value;
        }
        std::vector<TreeNode*> &getChildren(){
            return children;
        }
    private:
        std::string directive;
        std::string value;
        std::vector<TreeNode*> children;
};


std::string removeComments(std::ifstream &file);
std::string trimLeftRight(const std::string &str); 
std::string removeEmptyLines(const std::string &input); 
std::string trim(const std::string &input);

#endif //CONFIGPARSER