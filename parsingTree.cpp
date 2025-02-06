#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>
#include <sstream>

void print() {

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

std::string trimWhiteSpaces(const std::string &input) {
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

int main(){
    
    std::ifstream file("./nginxConfFiles/crazy.conf");
    if (!file) {
        std::cerr << "Unable to open file nginx.conf" << std::endl;
        return 1;
    }
    std::string noComments, freshCut;
    
    std::string v = removeEmptyLines(freshCut = trimWhiteSpaces(noComments = removeComments(file)));; 
    

    TreeNode *head = new TreeNode("root", ""); 
    std::stack<TreeNode*> s;
    s.push(head);
    std::istringstream lineStream(v);
    std::string token; 
    std::vector<std::string> tokens; 


    int firstRound = 1; 
    int i = 0; 
    while(lineStream >> token)
    {

        tokens.push_back(token);
        if(tokens[i] != token)
        {
            std::cerr << "Fuori Fase : token number =>" << i << std::endl;
            break;
        }
        //direttive
        if(isValidDirective(tokens[i]))
        {
            std::string key;
            std::string value;

            //TODO parse
            std::string value1;
            std::getline(lineStream, value1);
            tokens.push_back(value1);
            i++;
            if(value1.at(value1.size() - 1) != ';'){
                std::cerr << "error: " << __FILE__ << ": direttiva non valida" << __LINE__ << std::endl;  
                break; 
            }
            TreeNode *directiveNode = new TreeNode(token, trimWhiteSpaces(value1));
            s.top()->add(directiveNode);
            i++;
            continue; 
        }
        //inizio blocco
        if(token == "http" || token == "server" || token == "location")
        {

            if ((token == "http" || token == "server")){ 
                
                printf("token => %s\n", token.c_str());
                std::string openblock;

                lineStream >> openblock;
                
                tokens.push_back(openblock);
                i++;

                if(openblock != "{") {
                    std::cerr << "Error: Invalid block start on line " << i << std::endl;
                    break;
                }
                TreeNode *configBlock = new TreeNode(token, "");
                s.top()->add(configBlock);
                s.push(configBlock);
                i++;
                continue;         
            }
            if(token == "location"){
                
                std::string tokenPath; 
                
                lineStream >> tokenPath;
                
                tokens.push_back(tokenPath);
                i++;

                std::string openblock;

                lineStream >> openblock;
                
                tokens.push_back(openblock);
                i++;

                if(openblock != "{") {
                    std::cerr << "Error: Invalid block start on line " << i << std::endl;
                    break;
                }

                TreeNode *configBlock = new TreeNode(token, tokenPath);
                s.top()->add(configBlock);
                s.push(configBlock);
                i++;
                continue;
            } 
        }

        //root[[server[dir1, dir2, dir3, dir4(location)[dir1,dir2....]], server[]]]]

        
        //fine blocco
        if(token == "}"){
            if (s.size() <= 1) {
                std::cerr << "Error: Unmatched closing brace on line " << __LINE__ << std::endl;
            } else {
                s.pop();  // Pop the s when we encounter a closing brace
            }
            i++;
            continue;
        }
    }

    if(s.size() > 1){
        std::cerr << "Non tutti i blocchi sono stati chiusi" << std::endl;
        return(1); 
    }

    head->print();

    return 0; 
}