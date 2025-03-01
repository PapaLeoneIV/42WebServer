#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <limits.h>

#include <map>
#include <stack>
#include <string>
#include <vector>

#include "../includes/Server.hpp"


typedef int (*FunctionPtr)(std::vector<std::string>);
 
typedef std::map<std::string, std::vector<std::string> > ConfigDirectiveMap;

class TreeNode {
public:
  TreeNode(std::string directive, std::vector<std::string> value) {
    this->directive = directive;
    this->value = value;
  }
  void add(TreeNode *node) { children.push_back(node); }
  void print(int level = 0) {
    for (int i = 0; i < level; ++i)
      std::cout << "  ";
    std::cout << directive;
    if (!value.empty()) {
      size_t i = 0;
      while (i < value.size()) {
        std::cout << " " << value[i];
        i++;
      }
    }
    std::cout << std::endl;
    for (size_t i = 0; i < children.size(); ++i)
      children[i]->print(level + 1);
  }
  std::string &getDirective() { return directive; }
  std::vector<std::string> &getValue() { return value; }
  std::vector<TreeNode *> &getChildren() { return children; }

private:
  std::string directive;
  std::vector<std::string> value;
  std::vector<TreeNode *> children;
};

class ConfigParser {
public:
  int validatePath(std::string path);
  TreeNode *createConfigTree(std::string path);
  int isValidDirective(std::string token);
  bool verifyDirectives(Server *server);
  int checkMandatoryDirectives(Server *server);
  void extractDirectives(Server *server, TreeNode *node);




int parseListenValues(std::vector<std::string> v);
  int parseHostValues(std::vector<std::string> v);
  int parseServerNameValues(std::vector<std::string> v);
  int parseErrorPageValues(std::vector<std::string> v);
  int parseClientMaxBodyValues(std::vector<std::string> v);
  int parseRootValues(std::vector<std::string> v);
  int parseIndexValues(std::vector<std::string> v);
  int parseAutoIndexValues(std::vector<std::string> v);
  int parseAllowMethodsValues(std::vector<std::string> v);
  int parseReturnValues(std::vector<std::string> v);
  int parseAliasValues(std::vector<std::string> v);
  int parseCgiExtValues(std::vector<std::string> v);
  int parseCGIPathValues(std::vector<std::string> v);
  int parseProxyPassValues(std::vector<std::string> v);

  void setFileName(std::string path);
  std::string getFileName();

  ConfigParser();
  ~ConfigParser();

  std::map<std::string, int (ConfigParser::*)(std::vector<std::string>)> fnToParseDirectives;
  std::vector<std::string> directives;
  std::vector<std::string> extensionsAllowd;

private:
  std::string fileName;
};

std::string removeComments(std::ifstream &file);
std::string trimLeftRight(const std::string &str);
std::string removeEmptyLines(const std::string &input);
std::string trimm(const std::string &input);
int setUpDefaultDirectiveValues(Server *server);

#endif // CONFIGPARSER