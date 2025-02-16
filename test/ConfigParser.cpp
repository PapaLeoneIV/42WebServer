#include "ConfigParser.hpp"
#include <sstream>


#define push_back_next_token(token, tokenIdx)   \
                    lineStream >> token;    \
                    tokens.push_back(token);    \
                    tokenIdx++;

#define add_block_to_Tree(node, token, value, tokenIdx, s)  \
                    TreeNode *node = new TreeNode(token, value);    \
                    s.top()->add(node); \
                    tokenIdx++;

#define check_open_block(openblock, i)  \
                    if(openblock != "{") {  \
                        Logger::error(path, "is not a valid closing brace");  \
                        break;  \
                    }

#define check_directive_semicolom(value, tokenIdx)  \
                    if(value.at(value.size() - 1) != ';'){  \
                        Logger::error(path, "direttive needs to end with \";\" ");  \
                        break;  \
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

std::string trimm(const std::string &input) {
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

TreeNode * ConfigParser::parseConfigFile(std::string path){
    
    std::string file_name = std::string(path);
    std::ifstream file(file_name.c_str());
    if (!file) {
        std::cerr << "Unable to open file "<< path << std::endl;
        return NULL;
    }

    std::string noComments, trimmed;
    std::string v = removeEmptyLines(trimmed = trimm(noComments = removeComments(file)));; 
    std::istringstream lineStream(v);


    TreeNode *root = new TreeNode("root", (std::vector<std::string>)0); 
    std::stack<TreeNode*> s;
    s.push(root);
    

    std::vector<std::string> tokens; 
    std::string token; 


    int tokenIdx = 0; 
    while(lineStream >> token){
    
        tokens.push_back(token);
    
        if(tokens[tokenIdx] != token){
            Logger::error(path, "non valid token found");
            break;
        }
    
        //direttive
        if(isValidDirective(tokens[tokenIdx])){
            std::string value;

            //TODO check if value can be used for the token(directive) we are parsing atm-----> decide if we should do it after parsing
            //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/22
            std::getline(lineStream, value);
            std::istringstream valueStream(value);
            std::vector<std::string> valueTokens;
            std::string valueToken;
            while (valueStream >> valueToken) {
                valueTokens.push_back(trimLeftRight(valueToken));
                tokens.push_back(trimLeftRight(valueToken));
                tokenIdx++;
            }
            
            check_directive_semicolom(trimLeftRight(value), tokenIdx);

            add_block_to_Tree(directive, token, valueTokens, tokenIdx, s);
            continue; 
        }
        //inizio blocco
        if (token == "server"){ 

            std::string openblock;
            push_back_next_token(openblock, tokenIdx);
            check_open_block(openblock, tokenIdx);

            if(s.top()->getDirective() != "root") break; //server allowed only inside root node
            
            add_block_to_Tree(configBlock, token, (std::vector<std::string>)0 , tokenIdx, s);
            s.push(configBlock);
            continue;         
        }
        //inzio location block
        if(token == "location"){
            std::vector<std::string> locationPathV;
            std::string locationPath;
            push_back_next_token(locationPath, tokenIdx);
            std::string openblock;
            push_back_next_token(openblock, tokenIdx)
            check_open_block(openblock, tokenIdx);
            if(s.top()->getDirective() != "server") break;
            locationPathV.push_back(locationPath);
            add_block_to_Tree(configBlock, token, locationPathV, tokenIdx, s);
            s.push(configBlock);
            continue;
        }

        //fine blocco
        if(token == "}"){
            if (s.size() <= 1) {
                Logger::error(path,  "Error during the parsing");
                break;
            } else { s.pop(); }
            tokenIdx++;
            continue;
        }
    }

    if(s.size() > 1){
        // Logger::error(path,  "Error during the parsing");
        exit(1); 
    }
    return root;
}

/**
 * [address]?:[port]
 * 
 * 'address' might not be specified
 */

int parseListenValues(std::vector<std::string> v){
    if(v.size() > 1) return 1;
    std::string value = v[0]; 
    size_t columnIdx = value.find(":");
    //if ':' is present
    if(columnIdx == 0 || columnIdx == value.size()) return 1; 
    if(columnIdx != std::string::npos){
        std::string host = value.substr(0, columnIdx);
        
        std::string octect; 
        std::stringstream ss(value);
        while(getline(ss, octect, '.')){
            if(octect.empty() || octect.size() > 3) return 1; 
            
            size_t i = 0; 
            while(i < octect.size()){
                if(!isdigit(octect[i])) return 1;
                i++;
            }
            std::stringstream num(octect);
            int octectNum;
            num >> octectNum;
            if(octectNum < 0 ||octectNum > 255) return 1;
        }
        //if not
        std::string port = value.substr(columnIdx + 1, value.size());
        if(port.empty() || port.size() > 4) return 1;
         
        for(size_t i = 0; i < port.size(); ++i){
            if(!isdigit(port[i])) return 1; 
        }
        std::stringstream num(port);
        int portNum;
        num >> portNum;

        if(portNum < 0 || portNum > 65535) return 1; 
    }
    return 0; 
}
int parseHostValues(std::vector<std::string> v){
    if(v.size() > 1) return 1;
    std::string value = v[0];
    std::string octect; 
    std::stringstream ss(value);
    while(getline(ss, octect, '.')){
        if(octect.empty() || octect.size() > 3) return 1; 
        
        size_t i = 0; 
        while(i < octect.size()){
            if(!isdigit(octect[i])) return 1;
            i++;
        }
        std::stringstream num(octect);
        int octectNum;
        num >> octectNum;
        if(octectNum < 0 ||octectNum > 255) return 1;
    }
    return 0;
}

// TODO: implement some more robust checks
// Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/25
int parseServerNameValues(std::vector<std::string> v){
    size_t i = 0;
    while(i < v.size()){
        if(v[i].empty()) return 1;
        i++;
    }
    return 0;
}

int parseErrorPageValues(std::vector<std::string> v){
    if(v.size() != 2) return 1;
    std::string code = v[0];
    std::string path = v[1];
    if(code.size() != 3) return 1;
    for(size_t i = 0; i < code.size(); ++i){
        if(!isdigit(code[i])) return 1;
    }
    std::stringstream num(code);
    int codeNum;
    num >> codeNum;
    if(codeNum < 100 || codeNum > 599) return 1;

    if(path.empty()) return 1;
    if(path[0] != '/') return 1;
    if(access(path.c_str(), R_OK) != 0) return 1;

    return 0;
}

int parseClientMaxBodyValues(std::vector<std::string> v){
    if(v.size() != 1) return 1;
    std::string value = v[0];
    if(value.empty()) return 1;
    for(size_t i = 0; i < value.size(); ++i){
        if(!isdigit(value[i])) return 1;
    }
    std::stringstream num(value);
    int valueNum;
    num >> valueNum;
    if(valueNum <= 0 || valueNum >= INT_MAX) return 1;
    return 0;
}
//Syntax:	root path;
int parseRootValues(std::vector<std::string> v){
    if(v.size() != 1) return 1;
    std::string path = v[0];
    if(path.empty()) return 1;
    if(path[0] != '/') return 1;
    if(access(path.c_str(), R_OK) != 0) return 1;
    return 0;
}

//Syntax:	index file [file ...];
int parseIndexValues(std::vector<std::string> v){
    size_t i = 0;
    while(i < v.size()){
        std::string value = v[i];
        if(value.empty()) return 1;
        
        size_t dotIdx = value.find_last_of(".");
        if(dotIdx == 0 || dotIdx == value.size()) return 1;

        if(dotIdx != std::string::npos){
            std::string extension = value.substr(dotIdx, value.size());
            if(extension != ".html") return 1; // TODO: atm i m only accepting .html as index file, check if we can allow other extension
                                               // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/24
            i++;
        }
    }
    return 0;
}

//Syntax:	autoindex on | off;
int parseAutoIndexValues(std::vector<std::string> v){
    if(v.size() != 1) return 1;
    std::string value = v[0];
    if(value.empty()) return 1;
    
    if(value != "on" || value != "off") return 1;

    return 0;
}

int parseAllowMethodsValues(std::vector<std::string> v){
    if(v.empty() || v.size() > 5) return 1;
    size_t i = 0;
    while(i < v.size()){
        std::string value = v[0];
        if(value != "GET" && value != "POST" 
            && value != "PUT" && value != "DELETE" && value != "HEAD")
                return 1;
    }
    return 0;
}


//'return' code [text];
int parseReturnValues(std::vector<std::string> v){
    if(v.empty() || v.size() > 2) return 1;
    
    //error code mandatory if return is present
    std::string errorCode = v[0];

    for(size_t i = 0; i < errorCode.size(); ++i){
        if(!isdigit(errorCode[i])) return 1;
    }   
    std::stringstream num(errorCode);
    int codeNum;
    num >> codeNum; 
    if(codeNum < 100 || codeNum > 599) return 1;
    
    //parse text/url if present
    if(v.size() == 2){
        std::string text = v[1];
        if(text.empty()) return 1;
        //it needs to be a string enclose by double quotes(I DECIDED LIKE THIS OK?) lil bitch sit down
        if(text[0] != '\"' || text[text.size()] != '\"') return 1;
    }
    return 0;
}


//Syntax: alias path;
int parseAliasValues(std::vector<std::string> v){
    if(v.size() != 1) return 1;
    std::string path = v[0];
    if(path.empty()) return 1;
    if(path[0] != '/') return 1;
    if(access(path.c_str(), R_OK) != 0) return 1;
    return 0;
}

int parseCgiExtValues(std::vector<std::string> v){
    std::vector<std::string> extensionsAllowd;
    extensionsAllowd.push_back(".py");      // pyhton
    extensionsAllowd.push_back(".sh");      // bash
    extensionsAllowd.push_back(".cpp");     // c++
    extensionsAllowd.push_back(".c");       // c
    extensionsAllowd.push_back(".js");      // javascript
    extensionsAllowd.push_back(".ts");      // typescript
    extensionsAllowd.push_back(".pl");      // perl
    extensionsAllowd.push_back(".java");    // java
    extensionsAllowd.push_back(".php");     // php
    extensionsAllowd.push_back(".go");      // golang
    extensionsAllowd.push_back(".rs");      // rust
    extensionsAllowd.push_back(".hs");      // haskell

    if(v.size() < 1) return 1;
    size_t i = 0;
    while(i < v.size()){
        std::string extension = v[i];
        if(extension[0] != '.') return 1;
        size_t j = 0;
        while(j < extensionsAllowd.size()) return 1;
    }
    return 0;
}

int parseCGIPathValues(std::vector<std::string> v){
    if(v.size() != 1) return 1;
    std::string path = v[0];
    if(path.empty()) return 1;
    if(path[0] != '/') return 1;
    if(access(path.c_str(), R_OK) != 0) return 1;
    return 0;
}


//Syntax: proxy_pass URL;
int parseProxyPassValues(std::vector<std::string> v){
    if(v.size() != 1) return 1;
    std::string url = v[0];
    if(url.empty()) return 1;
    if(url[0] != '/') return 1;
    return 0;
}

int setUpDefaultValues(Server *server){
    //port 80 by default
    if(server->getServerDir()["listen"].empty()){
        server->setServerDir("listen", "80");
    }
    //host or 127.0.0.1 by default
    if(server->getServerDir()["host"].empty()){
        server->setServerDir("host", "127.0.0.1");
    }
    //default page when requesting a directory, index.html by default
    if(server->getServerDir()["index"].empty()){
        server->setServerDir("index", "index.html");
    }
    //allowed methods in location, GET only by default
    std::map<std::string, std::map<std::string, std::vector<std::string> > > locationDirectives = server->getLocationDir();
    for(std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = locationDirectives.begin(); it != locationDirectives.end(); ++it){
        if(it->second["allow_methods"].empty()){
            it->second["allow_methods"].push_back("GET");
        }
    }
    //root folder of the location, if not specified, taken from the server. 
    for(std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = locationDirectives.begin(); it != locationDirectives.end(); ++it){{
        if(it->second["root"].empty()){
            it->second["root"].push_back(server->getServerDir()["root"]);
        }
    }
    //default page when requesting a directory, copies root index by default
    for(std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = locationDirectives.begin(); it != locationDirectives.end(); ++it){
        if(it->second["index"].empty()){
            it->second["index"].push_back(server->getServerDir()["index"]);
            }
        }
    }   
}

int checkMandatoryDirectives(Server *server){
    std::vector<std::string> mandatoryServerDirectives;
    std::vector<std::string> mandatoryCGIDirectives;

    mandatoryServerDirectives.push_back("listen");
    mandatoryServerDirectives.push_back("root");

    mandatoryCGIDirectives.push_back("cgi_ext");
    mandatoryCGIDirectives.push_back("cgi_path");
    mandatoryCGIDirectives.push_back("root");

    std::map<std::string, std::vector<std::string> > serverDirectives = server->getServerDir();
    std::map<std::string, std::map<std::string, std::vector<std::string> > > locationDirectives = server->getLocationDir();

    std::vector<std::string>::iterator Mandatoryit;
    std::map<std::string, std::vector<std::string> >::iterator Serverit;
    std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator Locationit;
    


    bool found = false;
    for(Mandatoryit = mandatoryServerDirectives.begin(); Mandatoryit != mandatoryServerDirectives.end(); ++Mandatoryit){
        found = false;
        for(Serverit = serverDirectives.begin(); Serverit != serverDirectives.end(); ++Serverit){
            if(Serverit->first == *Mandatoryit){
                found = true;
            }
            
        }
        if(found == false){
            Logger::error("", "Error: missing mandatory directive: " + *Mandatoryit);
            return 1;
        }
    }
    found = false;
    for(Mandatoryit = mandatoryCGIDirectives.begin(); Mandatoryit != mandatoryCGIDirectives.end(); ++Mandatoryit){
        for(Locationit = locationDirectives.begin(); Locationit != locationDirectives.end(); ++Locationit){
            
            
            if(Locationit->first == "/cgi-bin"){
                found = false;    
                for(Serverit = Locationit->second.begin(); Serverit != Locationit->second.end(); ++Serverit){
                        if(Serverit->first == *Mandatoryit){
                            found = true;
                        }
                }
                if(found == false){
                    Logger::error("", "Error: missing mandatory directive: " + *Mandatoryit);
                    return 1;
                }
            }
        }
       
    }
    return 0;
}
ConfigParser::ConfigParser(){
    fnToParseDirectives["listen"] = parseListenValues;
    fnToParseDirectives["host"] = parseHostValues;
    fnToParseDirectives["server_name"] = parseServerNameValues;
    fnToParseDirectives["error_page"] = parseErrorPageValues;
    fnToParseDirectives["client_max_body_size"] = parseClientMaxBodyValues;
    fnToParseDirectives["root"] = parseRootValues;
    fnToParseDirectives["index"] = parseIndexValues;
    fnToParseDirectives["autoindex"] = parseAutoIndexValues;
    fnToParseDirectives["allow_methods"] = parseAllowMethodsValues;
    fnToParseDirectives["return"] = parseReturnValues;
    fnToParseDirectives["alias"] = parseAliasValues;
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