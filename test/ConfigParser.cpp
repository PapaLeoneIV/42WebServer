#include "ConfigParser.hpp"
#define push_back_next_token(token, tokenIdx)                                                                                  \
                    lineStream >> token;                                                                                \
                    tokens.push_back(token);                                                                            \
                    tokenIdx++;

#define add_block(node, token, value, tokenIdx, s)                                                                             \
                    TreeNode *node = new TreeNode(token, trim(value));                                                  \
                    s.top()->add(node);                                                                                 \
                    tokenIdx++;

#define check_open_block(openblock, i)                                                                                  \
                    if(openblock != "{") {                                                                              \
                        error(path, tokens, tokenIdx, "is not a valid closing brace");                                      \
                        break;                                                                                          \
                    }

#define check_directive_semicolom(value, tokenIdx)                                                                             \
                    if(value.at(value.size() - 1) != ';'){                                                              \
                        error(path, tokens, -1, "direttive needs to end with \";\" ");                                  \
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

std::string trimLeftRight(const std::string &str) {
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


int ConfigParser::isValidDirective(std::string token){
    std::vector<std::string>::iterator it;
    for(it = this->directives.begin(); it != this->directives.end(); ++it){
        if(token == *it)
            return 1;
    }
    return 0;
}


void error(std::string file, std::vector<std::string> tokens, int tokenIdx , std::string msg){
    if (tokenIdx > -1){
        std::cerr << "\e[1m" << file  << ": \e[31merror: " 
        << "\e[0m\e[1m\"" << tokens[tokenIdx] << "\" \e[0m" << msg << std::endl; 
        return;  
    }
    std::cerr << "\e[1m" << file << ":token id[" << tokenIdx  << "]: \e[31merror: " 
        << "\e[0m\e[1m\e[0m" << msg << std::endl; 
        return;
}

int ConfigParser::validateConfigPath(std::string path) {

    if (path.empty()) {
        std::cerr << "Error: Path is empty." << std::endl;
        return 1;
    }

    int dotIdx = path.find_last_of(".");
    std::string fileExtension = path.substr(dotIdx, path.size());
    if (fileExtension != ".conf") {
        std::cerr << "Error: The file must have a .conf extension." << std::endl;
        return 1;
    }

    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) != 0 || !(fileInfo.st_mode & S_IFREG)) {
        std::cerr << "Error: The file does not exist or is not a regular file." << std::endl;
        return 1;
    }

    if (access(path.c_str(), R_OK) != 0) {
        std::cerr << "Error: The file is not readable." << std::endl;
        return 1;
    }

    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: The file could not be opened." << std::endl;
        return 1;
    }
    return 0;
}

int ConfigParser::parseConfigFile(std::string path){
    
    std::string file_name = std::string(path);
    std::ifstream file(file_name.c_str());
    if (!file) {
        std::cerr << "Unable to open file "<< path << std::endl;
        return 1;
    }

    std::string noComments, trimmed;
    std::string v = removeEmptyLines(trimmed = trim(noComments = removeComments(file)));; 
    std::istringstream lineStream(v);


    TreeNode *root = new TreeNode("root", ""); 
    std::stack<TreeNode*> s;
    s.push(root);
    

    std::vector<std::string> tokens; 
    std::string token; 


    int tokenIdx = 0; 
    while(lineStream >> token){
    
        tokens.push_back(token);
    
        if(tokens[tokenIdx] != token){
            error(path, tokens, tokenIdx , "is not a valid token");
            break;
        }
    
        //direttive
        if(isValidDirective(tokens[tokenIdx])){
            std::string value;

            //TODO check if value can be used for the token(directive) we are parsing atm-----> decide if we should do it after parsing
            //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/22
            std::getline(lineStream, value);
            tokens.push_back(trimLeftRight(value));
            tokenIdx++;

            check_directive_semicolom(trimLeftRight(value), tokenIdx);

            add_block(directive, token, trimLeftRight(value), tokenIdx, s);
            continue; 
        }
        //inizio blocco
        if (token == "server"){ 

            std::string openblock;
            push_back_next_token(openblock, tokenIdx);
            check_open_block(openblock, tokenIdx);
            if(s.top()->getDirective() != "root") break;
            add_block(configBlock, token, "", tokenIdx, s);
            s.push(configBlock);
            continue;         
        }
        //inzio location block
        if(token == "location"){
            std::string locationPath;
            push_back_next_token(locationPath, tokenIdx);
            std::string openblock;
            push_back_next_token(openblock, tokenIdx)
            check_open_block(openblock, tokenIdx);
            if(s.top()->getDirective() != "server") break;
            add_block(configBlock, token, locationPath, tokenIdx, s);
            s.push(configBlock);
            continue;
        }

        //fine blocco
        if(token == "}"){
            if (s.size() <= 1) {
                error(path, tokens, tokenIdx, "Error during the parsing");
                break;
            } else { s.pop(); }
            tokenIdx++;
            continue;
        }
    }

    if(s.size() > 1){
        error(path, tokens, tokenIdx, "Error during the parsing");
        return(1); 
    }
    return 0;
}


int parseListenValues(std::vector<std::string> v){
    
}

ConfigParser::ConfigParser(){
    fnToParseDirectives["listen"] = parseListenValues;
    fnToParseDirectives["host"] = parseHostValues;
    fnToParseDirectives["server_name"] = parseServerNameValues;
    fnToParseDirectives["error_page"] = parseErrorPageValues;
    fnToParseDirectives["client_max_body_size"] = parseCMAXBODYValues;
    fnToParseDirectives["root"] = parseRootValues;
    fnToParseDirectives["index"] = parseIndexValues;
    fnToParseDirectives["autoindex"] = parseAutoIndexValues;
    fnToParseDirectives["allow_methods"] = parseAllowMethodsValues;
    fnToParseDirectives["return"] = parseReturnValues;
    fnToParseDirectives["alias"] = parseAlisValues;
    fnToParseDirectives["cgi_ext"] = parseCgiExtValues;
    fnToParseDirectives["cgi_path"] = parseCGIPathValues;
    fnToParseDirectives["proxy_pass"] = parseProxyPassValues;




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
    directives.push_back("proxy_pass");
};
ConfigParser::~ConfigParser(){};