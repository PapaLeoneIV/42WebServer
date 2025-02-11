#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>
#include <sstream>
#include <map>

#define push_back_next_token(token, i)                                                                                  \
                    lineStream >> token;                                                                                \
                    tokens.push_back(token);                                                                            \
                    i++;

#define add_block(node, token, value, i, s)                                                                             \
                    TreeNode *node = new TreeNode(token, trim(value));                                                  \
                    s.top()->add(node);                                                                                 \
                    i++;

#define check_open_block(openblock, i)                                                                                  \
                    if(openblock != "{") {                                                                              \
                        error(argv[1], tokens, i, "is not a valid closing brace");                                      \
                        break;                                                                                          \
                    }

#define check_directive_semicolom(value, i)                                                                             \
                    if(value.at(value.size() - 1) != ';'){                                                              \
                        error(argv[1], tokens, -1, "direttive needs to end with \";\" ");                               \
                        break;                                                                                          \
                    }

std::string removeComments(std::ifstream &file) {
    std::string line, result;
    while (std::getline(file, line)) {
        size_t commentPos = line.find("#");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);  
        }
        result += line + "\n";
    }
    return result;
}

std::string trimOriginal(const std::string &str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

std::string trim(const std::string &input) {
    std::istringstream inputStream(input);
    std::ostringstream resultStream;
    std::string line;

    while (std::getline(inputStream, line)) {
        std::istringstream lineStream(line);
        std::string word;
        bool firstWord = true;
        while (lineStream >> word) {
            if (!firstWord) {
                resultStream << " ";
            }
            resultStream << word;
            firstWord = false;
        }
        resultStream << "\n";
    }
    return resultStream.str();
}

std::string removeEmptyLines(const std::string &input) {
    std::istringstream inputStream(input);
    std::ostringstream resultStream;
    std::string line;

    while (std::getline(inputStream, line)) {
        size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
            ++i;
        }
        if (i < line.size()) {  
            resultStream << line << "\n";
        }
    }
    return resultStream.str();
}

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


int isValidDirective(std::string token){
    std::vector<std::string> directives;
    directives.push_back("listen");
    directives.push_back("host");
    directives.push_back("server_name");
    directives.push_back("error_page");
    directives.push_back("client_max_body_size");
    directives.push_back("root");
    directives.push_back("index");
    directives.push_back("autoindex");
    directives.push_back("allow_methods");
    directives.push_back("return");
    directives.push_back("alias");
    directives.push_back("cgi_ext");
    directives.push_back("cgi_path");


    std::vector<std::string>::iterator it; 
    for(it = directives.begin(); it != directives.end(); ++it)    
    {
        if(token == *it)
            return 1;
    }
    return 0;
}


void error(std::string file, std::vector<std::string> tokens, int i , std::string msg){
    if (i > -1){
        std::cerr << "\e[1m" << file  << ": \e[31merror: " 
        << "\e[0m\e[1m\"" << tokens[i] << "\" \e[0m" << msg << std::endl; 
        return;  
    }
    std::cerr << "\e[1m" << file << ":token id[" << i  << "]: \e[31merror: " 
        << "\e[0m\e[1m\e[0m" << msg << std::endl; 
        return;
}

void traverseTree(Server *server, TreeNode *node){
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
                server->setLocationDir(node->getValue(), (*currentNode)->getDirective(), (*currentNode)->getValue());
            }
        }
        traverseTree(*it, serverDir, locationDir);
    }
}


int main(int argc, char *argv[]) {
    
    if(argc != 2){
        std::cerr << "Usage: " << argv[0] << " <config-file>" << std::endl;
        return 1;
    }
    std::string file_name = std::string("./config/") + std::string(argv[1]);
    std::ifstream file(file_name.c_str());
    if (!file) {
        std::cerr << "Unable to open file "<< argv[1] << std::endl;
        return 1;
    }
    std::string noComments, trimmed;
    std::string v = removeEmptyLines(trimmed = trim(noComments = removeComments(file)));; 
    

    TreeNode *head = new TreeNode("root", ""); 
    std::stack<TreeNode*> s;
    s.push(head);
    std::istringstream lineStream(v);
    std::vector<std::string> tokens; 
    std::string token; 


    int i = 0; 
    while(lineStream >> token)
    {
        tokens.push_back(token);
        if(tokens[i] != token)
        {
            error(argv[1], tokens, i , "is not a valid token");
            break;
        }
        //direttive
        if(isValidDirective(tokens[i]))
        {
            std::string value;

            //TODO check if value can be used for the token(directive) we are parsing atm-----> decide if we should do it after parsing
            //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/22
            std::getline(lineStream, value);
            tokens.push_back(trimOriginal(value));
            i++;

            check_directive_semicolom(trimOriginal(value), i);
            
            add_block(directive, token, trimOriginal(value), i, s);
            continue; 
        }
        //inizio blocco
        if (token == "server"){ 
            
            std::string openblock;
            push_back_next_token(openblock, i);
            
            check_open_block(openblock, i);
        
            add_block(configBlock, token, "", i, s);
            
            s.push(configBlock);
            //Server server = new Server();
            
            continue;         
        }
        //inzio location block
        if(token == "location"){
            
            std::string tokenPath;
            push_back_next_token(tokenPath, i);

            std::string openblock;
            push_back_next_token(openblock, i)
            
            check_open_block(openblock, i);
            
            add_block(configBlock, token, tokenPath, i, s);
            
            s.push(configBlock);
            continue;
        }

        //fine blocco
        if(token == "}"){
            if (s.size() <= 1) {
                error(argv[1], tokens, i, "Error during the parsing");
                break;
            } else {
                s.pop();  // Pop the stack when we encounter a closing brace
            }
            i++;
            continue;
        }
    }
    if(s.size() > 1){
        error(argv[1], tokens, i, "Error during the parsing");
        return(1); 
    }
    
  //head->print();

  // std::map<std::string, std::string> serverDir;
  // std::map<std::string, std::map<std::string, std::string> > locationDir;

    std::vector<TreeNode*> children = head->getChildren();
    std::vector<TreeNode*>::iterator currentNode;
    ServerManager serverManager = new ServerManager();
    for(currentNode = children.begin(); currentNode != children.end(); ++currentNode){
      if(currentNode->getDirective() == "server"){
          Server server = new Server();
          traverseTree(server, currentNode);
          serverManager->addServer(server);
      }
    }



    return 0; 
}