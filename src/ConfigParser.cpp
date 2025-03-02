#include "ServerManager.hpp"
#include "ConfigParser.hpp"
#include "Booter.hpp"
#include "Exception.hpp"
#include "Logger.hpp"
#include <sstream>

#define push_back_next_token(token, tokenIdx) \
    lineStream >> token;                      \
    tokens.push_back(token);                  \
    tokenIdx++;

#define add_block_to_Tree(node, token, value, tokenIdx, s) \
    TreeNode *node = new TreeNode(token, value);           \
    s.top()->add(node);                                    \
    tokenIdx++;

#define check_open_block(openblock, i)                                                \
    if (openblock != "{")                                                             \
    {                                                                                 \
        Logger::error(path, "'server' || 'location' needs to have a open brace '{'"); \
        return NULL;                                                                  \
    }

#define check_directive_semicolom(value, tokenIdx)               \
    if (value.at(value.size() - 1) != ';')                       \
    {                                                            \
        Logger::error(path, "directive needs to end with ';' "); \
        return NULL;                                             \
    }

/** This function uses fnToParseDirectives, its a map of string and functions.
    The associated functions are used to parse the values of the directives.
    ex:
    fnToParseDirectives["listen"] = &ConfigParser::parseListenValues;
    or
    fnToParseDirectives["host"] = &ConfigParser::parseHostValues;
*/
bool ConfigParser::verifyDirectives(Server *server)
{
    ConfigDirectiveMap serverDir = server->getServerDir();
    for (ConfigDirectiveMap::iterator serverDirIt = serverDir.begin(); serverDirIt != serverDir.end(); ++serverDirIt)
    {
        std::string nginxDir = (*serverDirIt).first;
        std::vector<std::string> nginxDirValue = (*serverDirIt).second;
        if ((this->*fnToParseDirectives[nginxDir])(nginxDirValue))
            return 1;
    }
    std::map<std::string, ConfigDirectiveMap>::iterator locationDirIt;
    std::map<std::string, ConfigDirectiveMap> locationDir = server->getLocationDir();
    for (locationDirIt = locationDir.begin(); locationDirIt != locationDir.end(); ++locationDirIt)
    {
        ConfigDirectiveMap locationDirMap = (*locationDirIt).second;
        for (ConfigDirectiveMap::iterator locationDirMapIt = locationDirMap.begin(); locationDirMapIt != locationDirMap.end(); ++locationDirMapIt)
        {
            std::string nginxLocationDir = (*locationDirMapIt).first;
            std::vector<std::string> nginxLocationDirValue = (*locationDirMapIt).second;
            if ((this->*fnToParseDirectives[nginxLocationDir])(nginxLocationDirValue))
                return 1;
        }
    }
    return 0;
}

std::string removeComments(std::ifstream &file)
{
    std::string line, result;
    while (std::getline(file, line))
    {
        size_t commentPos = line.find("#");
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }
        result += line + "\n";
    }
    return result;
}

std::string trimLeftRight(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

std::string trimm(const std::string &input)
{
    std::istringstream inputStream(input);
    std::ostringstream resultStream;
    std::string line;

    while (std::getline(inputStream, line))
    {
        std::istringstream lineStream(line);
        std::string word;
        bool firstWord = true;
        while (lineStream >> word)
        {
            if (!firstWord)
            {
                resultStream << " ";
            }
            resultStream << word;
            firstWord = false;
        }
        resultStream << "\n";
    }
    return resultStream.str();
}

std::string removeEmptyLines(const std::string &input)
{
    std::istringstream inputStream(input);
    std::ostringstream resultStream;
    std::string line;

    while (std::getline(inputStream, line))
    {
        size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
        {
            ++i;
        }
        if (i < line.size())
        {
            resultStream << line << "\n";
        }
    }
    return resultStream.str();
}

/** This function is used to check if the directive is valid.
    It uses the directives vector to check if the directive is valid.
    If the directive is valid it returns 1, or there are two possibilities. One that the
    directive is not valid, in which case will be noticed by ConfigParser later. Or the directive
    is 'server' or 'location'.
    */
int ConfigParser::isValidDirective(std::string token)
{
    std::vector<std::string>::iterator it;
    for (it = this->directives.begin(); it != this->directives.end(); ++it)
    {
        if (token == *it)
        {
            return 1;
        }
    }
    return 0;
}

int ConfigParser::validatePath(std::string path)
{
    if (path.empty())
    {
        Logger::error(path, "Path is empty.");
        return 1;
    }

    size_t dotIdx = path.find_last_of(".");
    if (dotIdx == std::string::npos)
    {
        Logger::error(path, "The file must have a '.' to indicate the file extension.");
        return 1;
    }
    std::string fileExtension = path.substr(dotIdx, path.size());
    if (fileExtension != ".conf")
    {
        Logger::error(path, "The file must have a .conf extension.");
        return 1;
    }

    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) != 0 || !(fileInfo.st_mode & S_IFREG))
    {
        Logger::error(path, "The file does not exist or is not a regular file.");
        return 1;
    }

    if (access(path.c_str(), R_OK) != 0)
    {
        Logger::error(path, "The file is not readable.");
        return 1;
    }

    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        Logger::error(path, "The file could not be opened.");
        return 1;
    }

    this->setFileName(path);
    return 0;
}

int ConfigParser::fromConfigFileToServers(char *file)
{

    Booter booter;
    std::vector<Server *> servers;

    TreeNode *root = this->createConfigTree(std::string(file));

    if (root == NULL){
        Logger::error("", "invalid configuration file");
        return 1;
    }

    for (std::vector<TreeNode *>::iterator currentNode = root->getChildren().begin(); currentNode != root->getChildren().end(); ++currentNode)
    {
        if ((*currentNode)->getDirective() == "server")
        {
            Server *server = new Server();

            if(this->extractDirectives(server, *currentNode)){
                Logger::error(this->getFileName(), "duplicate directive error");
                return 1;
            }

            if (this->checkMandatoryDirectives(server)){
                Logger::error(this->getFileName(), "missing mandatory directives");
                return 1;
            }

            setUpDefaultDirectiveValues(server);

            if (this->verifyDirectives(server)){
                Logger::error(this->getFileName(), "invalid directive value");
                return 1;
            }
            servers.push_back(server); 
        }
    }
    this->setTmpServer(servers);
    return 0;
}

TreeNode *ConfigParser::createConfigTree(std::string path)
{

    std::ifstream file(path.c_str());
    if (!file)
    {
        Logger::error(path, "The file could not be opened.");
        return NULL;
    }

    std::string noComments, trimmed;
    std::string v = removeEmptyLines(trimmed = trimm(noComments = removeComments(file)));
    std::istringstream lineStream(v);

    TreeNode *root = new TreeNode("root", (std::vector<std::string>)0);
    std::stack<TreeNode *> s;
    s.push(root);

    std::vector<std::string> tokens;
    std::string token;

    int tokenIdx = 0;
    while (lineStream >> token)
    {

        tokens.push_back(token);

        if (tokens[tokenIdx] != token)
            return NULL;

        // direttive
        if (isValidDirective(tokens[tokenIdx]))
        {
            std::string value;
            std::getline(lineStream, value);
            std::istringstream valueStream(value);

            std::vector<std::string> valueTokens;
            std::string valueToken;
            while (valueStream >> valueToken)
            {
                valueToken = trimLeftRight(valueToken);
                valueTokens.push_back(valueToken);
                tokens.push_back(valueToken);
                tokenIdx++;
            }
            std::string &lastValue = valueTokens[valueTokens.size() - 1];
            check_directive_semicolom(lastValue, tokenIdx);
            lastValue = lastValue.substr(0, lastValue.size() - 1);
            add_block_to_Tree(directive, token, valueTokens, tokenIdx, s);
            continue;
        }
        // inizio blocco
        if (token == "server")
        {
            std::string openblock;
            push_back_next_token(openblock, tokenIdx);
            check_open_block(openblock, tokenIdx);

            if (s.top()->getDirective() != "root")
                break; // server allowed only inside root node

            add_block_to_Tree(configBlock, token, (std::vector<std::string>)0, tokenIdx, s);
            s.push(configBlock);
            continue;
        }
        // inzio location block
        if (token == "location")
        {
            std::vector<std::string> locationPathV;
            std::string locationPath;
            push_back_next_token(locationPath, tokenIdx);
            std::string openblock;
            push_back_next_token(openblock, tokenIdx)
                check_open_block(openblock, tokenIdx);
            if (s.top()->getDirective() != "server")
                break;
            locationPathV.push_back(locationPath);
            add_block_to_Tree(configBlock, token, locationPathV, tokenIdx, s);
            s.push(configBlock);
            continue;
        }

        // fine blocco
        if (token == "}")
        {
            if (s.size() <= 1)
            {
                Logger::error(path, "Error during the parsing");
                return NULL;
            }
            else
                s.pop();
            tokenIdx++;
            continue;
        }
        Logger::error(path, "invalid token found '" + token + "'");
    }

    if (s.size() > 1)
    {
        Logger::error(path, "Error during the parsing");
        return (NULL);
    }
    return root;
}

/**
 * [address]?:[port]
 *
 * 'address' might not be specified
 */

int ConfigParser::parseListenValues(std::vector<std::string> v)
{
    if (v.size() > 1)
    {
        Logger::error(this->getFileName(), "listen directive can't have more than one value");
        return 1;
    }
    std::string value = v[0];
    size_t columnIdx = value.find(":");
    // if ':' is present
    if (columnIdx == 0 || columnIdx == value.size())
    {
        Logger::error(this->getFileName(), "':' position cannot be at the beginning or at the end of the string");
        return 1;
    }
    if (columnIdx != std::string::npos)
    {
        std::string host = value.substr(0, columnIdx);

        std::string octect;
        std::stringstream ss(value);
        while (getline(ss, octect, '.'))
        {
            if (octect.empty() || octect.size() > 3)
            {
                Logger::error(this->getFileName(), "address octect is empty or has more than 3 characters");
                return 1;
            }

            size_t i = 0;
            while (i < octect.size())
            {
                if (!isdigit(octect[i]))
                {
                    Logger::error(this->getFileName(), "address octect is not a digit");
                    return 1;
                }
                i++;
            }
            std::stringstream num(octect);
            int octectNum;
            num >> octectNum;
            if (octectNum < 0 || octectNum > 255)
            {
                Logger::error(this->getFileName(), "address octect is not in the range 0-255");
                return 1;
            }
        }
    }
    // if not
    std::string port = value.substr(columnIdx + 1, value.size());
    if (port.empty() || port.size() > 4)
    {
        Logger::error(this->getFileName(), "'port' is empty or has more than 4 characters");
        return 1;
    }

    for (size_t i = 0; i < port.size(); ++i)
    {
        if (!isdigit(port[i]))
        {
            Logger::error(this->getFileName(), "'port' is not a digit");
            return 1;
        }
    }
    std::stringstream num(port);
    int portNum;
    num >> portNum;

    if (portNum < 0 || portNum > 65535)
    {
        Logger::error(this->getFileName(), "'port' is not in the range 0-65535");
        return 1;
    }
    return 0;
}
int ConfigParser::parseHostValues(std::vector<std::string> v)
{
    if (v.size() > 1)
    {
        Logger::error(this->getFileName(), "'host' directive can't have more than one value");
        return 1;
    }

    std::string value = v[0];
    std::string octect;
    std::stringstream ss(value);
    while (getline(ss, octect, '.'))
    {
        if (octect.empty() || octect.size() > 3)
        {
            Logger::error(this->getFileName(), "'host' octect is empty or has more than 3 characters");
            return 1;
        }

        size_t i = 0;
        while (i < octect.size())
        {
            if (!isdigit(octect[i]))
            {
                Logger::error(this->getFileName(), "'host' octect is not a digit");
                return 1;
            }
            i++;
        }
        std::stringstream num(octect);
        int octectNum;
        num >> octectNum;
        if (octectNum < 0 || octectNum > 255)
        {
            Logger::error(this->getFileName(), "'host' octect is not in the range 0-255");
            return 1;
        }
    }
    return 0;
}

// TODO: implement some more robust checks
// Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/25
int ConfigParser::parseServerNameValues(std::vector<std::string> v)
{
    size_t i = 0;
    while (i < v.size())
    {
        if (v[i].empty())
        {
            Logger::error(this->getFileName(), "server_name directive can't have empty values");
            return 1;
        }
        i++;
    }
    return 0;
}

int ConfigParser::parseErrorPageValues(std::vector<std::string> v)
{
    if (v.size() != 2)
    {
        Logger::error(this->getFileName(), "'error_page' directive must have 2 values [code] [path]");
        return 1;
    }
    std::string code = v[0];
    std::string path = v[1];
    if (code.size() != 3)
    {
        Logger::error(this->getFileName(), "'error_page' [code] must have 3 digits");
        return 1;
    }
    for (size_t i = 0; i < code.size(); ++i)
    {
        if (!isdigit(code[i]))
        {
            Logger::error(this->getFileName(), "'error_page' [code] must be a digit");
            return 1;
        }
    }
    std::stringstream num(code);
    int codeNum;
    num >> codeNum;
    if (codeNum < 100 || codeNum > 599)
    {
        Logger::error(this->getFileName(), "'error_page' [code] must be in the range 100-599");
        return 1;
    }

    if (path.empty())
    {
        Logger::error(this->getFileName(), "'error_page' [path] is empty");
        return 1;
    }
    std::string rootFolder = getcwd(NULL, 0);
    rootFolder = rootFolder.append("/");
    rootFolder = rootFolder.append(path);
    if (access(rootFolder.c_str(), R_OK) != 0)
    {
        Logger::error(this->getFileName(), "'error_page' [path] is not readable " + rootFolder);
        return 1;
    }
    return 0;
}

int ConfigParser::parseClientMaxBodyValues(std::vector<std::string> v)
{
    if (v.size() != 1)
    {
        Logger::error(this->getFileName(), "'client_max_body_size' directive must have 1 value");
        return 1;
    }
    std::string value = v[0];
    if (value.empty())
    {
        Logger::error(this->getFileName(), "'client_max_body_size' directive can't have empty values");
        return 1;
    }
    for (size_t i = 0; i < value.size(); ++i)
    {
        if (!isdigit(value[i]))
        {
            Logger::error(this->getFileName(), "'client_max_body_size' directive must be a digit");
            return 1;
        }
    }
    std::stringstream num(value);
    int valueNum;
    num >> valueNum;
    if (valueNum <= 0 || valueNum >= INT_MAX)
    {
        Logger::error(this->getFileName(), "'client_max_body_size' directive must be in the range 1-INT_MAX");
        return 1;
    }
    return 0;
}
// Syntax:	root path;
int ConfigParser::parseRootValues(std::vector<std::string> v)
{
    if (v.size() != 1)
    {
        Logger::error(this->getFileName(), "'root' path must have 1 value");
        return 1;
    }
    std::string path = v[0];
    if (path.empty())
    {
        Logger::error(this->getFileName(), "'root' path can't have empty values");
        return 1;
    }
    if (access(path.c_str(), R_OK) != 0)
    {
        Logger::error(this->getFileName(), "'root' path is not readable " + path);
        return 1;
    }
    return 0;
}

// Syntax:	index file [file ...];
int ConfigParser::parseIndexValues(std::vector<std::string> v)
{
    size_t i = 0;
    while (i < v.size())
    {
        std::string value = v[i];
        if (value.empty())
        {
            Logger::error(this->getFileName(), "'index' directive can't have empty values");
            return 1;
        }

        size_t dotIdx = value.find_last_of(".");
        if (dotIdx == 0 || dotIdx == value.size())
        {
            Logger::error(this->getFileName(), "'.' position cannot be at the beginning or at the end of the string");
            return 1;
        }

        if (dotIdx != std::string::npos)
        {
            std::string extension = value.substr(dotIdx, value.size());
            if (extension != ".html")
            {
                Logger::error(this->getFileName(), "index directive must have .html extension");
                return 1; // TODO: atm i m only accepting .html as index file, check if we can allow other extension
            }
            // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/24
            i++;
        }
    }
    return 0;
}

// Syntax:	autoindex on | off;
int ConfigParser::parseAutoIndexValues(std::vector<std::string> v)
{
    if (v.size() != 1)
    {
        Logger::error(this->getFileName(), "'autoindex' directive must have 1 value");
        return 1;
    }
    std::string value = v[0];
    if (value.empty())
    {
        Logger::error(this->getFileName(), "'autoindex' directive can't have empty values");
        return 1;
    }

    if (value != "on" && value != "off")
    {
        Logger::error(this->getFileName(), "'autoindex' directive must be 'on' or 'off'");
        return 1;
    }

    return 0;
}

int ConfigParser::parseAllowMethodsValues(std::vector<std::string> v)
{
    if (v.empty() || v.size() > 6)
    {
        Logger::error(this->getFileName(), "'allow_methods' directive must have 1-5 values");
        return 1;
    }
    size_t i = 0;
    while (i < v.size())
    {
        std::string value = v[0];
        if (value != "GET" && value != "POST" && value != "PUT" && value != "DELETE" && value != "HEAD")
        {
            Logger::error(this->getFileName(), "'allow_methods' directive must be 'GET', 'POST', 'PUT', 'DELETE' or 'HEAD'");
            return 1;
        }
        i++;
    }
    return 0;
}

//'return' code [text];
// TODO :need to attach root to the [text] value
int ConfigParser::parseReturnValues(std::vector<std::string> v)
{
    if (v.empty() || v.size() > 2)
    {
        Logger::error(this->getFileName(), "'return' directive must have 1-2 values");
        return 1;
    }

    // error code mandatory if return is present
    std::string errorCode = v[0];

    for (size_t i = 0; i < errorCode.size(); ++i)
    {
        if (!isdigit(errorCode[i]))
        {
            Logger::error(this->getFileName(), "'return' directive must have a digit as first value");
            return 1;
        }
    }
    std::stringstream num(errorCode);
    int codeNum;
    num >> codeNum;
    if (codeNum < 100 || codeNum > 599)
    {
        Logger::error(this->getFileName(), "'return' directive must be in the range 100-599");
        return 1;
    }

    // ConfigParser::parse text/url if present
    if (v.size() == 2)
    {
        std::string text = v[1];
        if (text.empty())
        {
            Logger::error(this->getFileName(), "'return' directive can't have empty values");
            return 1;
        }
        // it needs to be a string enclose by double quotes(I DECIDED LIKE THIS OK?) lil bitch sit down
        // if (text[0] != '\"' || text[text.size()] != '\"'){
        //     Logger::error(this->getFileName(), "return value must be enclosed in double queotes ");
        //     return 1;
        // }
    }
    return 0;
}

// Syntax: alias path;
int ConfigParser::parseAliasValues(std::vector<std::string> v)
{
    if (v.size() != 1)
    {
        Logger::error(this->getFileName(), "'alias' directive must have a value");
        return 1;
    }
    std::string path = v[0];
    if (path.empty())
    {
        Logger::error(this->getFileName(), "'alias' path cannot be empty");
        return 1;
    }
    return 0;
}

int ConfigParser::parseCgiExtValues(std::vector<std::string> v)
{
    std::vector<std::string> extensionsAllowd;
    extensionsAllowd.push_back(".py");   // pyhton
    extensionsAllowd.push_back(".sh");   // bash
    extensionsAllowd.push_back(".cpp");  // c++
    extensionsAllowd.push_back(".c");    // c
    extensionsAllowd.push_back(".js");   // javascript
    extensionsAllowd.push_back(".ts");   // typescript
    extensionsAllowd.push_back(".pl");   // perl
    extensionsAllowd.push_back(".java"); // java
    extensionsAllowd.push_back(".php");  // php
    extensionsAllowd.push_back(".go");   // golang
    extensionsAllowd.push_back(".rs");   // rust
    extensionsAllowd.push_back(".hs");   // haskell

    if (v.size() < 1)
    {
        Logger::error(this->getFileName(), "cgi_ext directive must have at least 1 value");
        return 1;
    }
    size_t i = 0;
    while (i < v.size())
    {
        std::string extension = v[i];
        if (extension[0] != '.')
        {
            Logger::error(this->getFileName(), "cgi_ext [extension] must start with '.'");
            return 1;
        }
        bool knwonExtension = true;
        for (size_t j = 0; j < extensionsAllowd.size(); ++j)
        {
            if (extension == extensionsAllowd[j])
                knwonExtension = false;
        }
        if (knwonExtension)
        {
            Logger::error(this->getFileName(), "cgi_ext [extension] is not allowed");
            return 1;
        }
        i++;
    }
    return 0;
}

int ConfigParser::parseCGIPathValues(std::vector<std::string> v)
{
    if (v.size() < 1)
    {
        Logger::error(this->getFileName(), "cgi_path directive must have at least 1 value");
        return 1;
    }
    for (size_t i = 0; i < v.size(); ++i)
    {
        std::string path = v[0];
        if (path.empty())
        {
            Logger::error(this->getFileName(), "cgi_path [path] can't have empty values");
            return 1;
        }
        if (path[0] != '/')
        {
            Logger::error(this->getFileName(), "cgi_path [path] must start with '/'");
            return 1;
        }
        if (access(path.c_str(), R_OK) != 0)
        {
            Logger::error(this->getFileName(), "cgi_path [path] is not readable");
            return 1;
        }
    }
    return 0;
}

// Syntax: proxy_pass URL;
int ConfigParser::parseProxyPassValues(std::vector<std::string> v)
{
    if (v.size() != 1)
    {
        Logger::error(this->getFileName(), "proxy_pass directive must have 1 value");
        return 1;
    }
    std::string url = v[0];
    if (url.empty())
    {
        Logger::error(this->getFileName(), "proxy_pass [URL] can't have empty values");
        return 1;
    }
    if (url[0] != '/')
    {
        Logger::error(this->getFileName(), "proxy_pass [URL] must start with '/'");
        return 1;
    }
    return 0;
}

int setUpDefaultDirectiveValues(Server *server)
{
    // port 80 by default
    if (server->getServerDir()["listen"].empty())
    {
        std::vector<std::string> tmp;
        tmp.push_back("80");
        server->setServerDir("listen", tmp);
    }
    // host or 127.0.0.1 by default
    if (server->getServerDir()["host"].empty())
    {
        std::vector<std::string> tmp;
        tmp.push_back("127.0.0.1");
        server->setServerDir("host", tmp);
    }
    // default page when requesting a directory, index.html by default
    if (server->getServerDir()["index"].empty())
    {
        std::vector<std::string> tmp;
        tmp.push_back("index.html");
        server->setServerDir("index", tmp);
    }
    // allowed methods in location, GET only by default
    std::map<std::string, std::map<std::string, std::vector<std::string> > > locationDirectives = server->getLocationDir();
    for (std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = locationDirectives.begin(); it != locationDirectives.end(); ++it)
    {
        if (it->second["allow_methods"].empty())
        {
            it->second["allow_methods"].push_back("GET");
        }
    }
    // root folder of the location, if not specified, taken from the server.
    for (std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = locationDirectives.begin(); it != locationDirectives.end(); ++it)
    {
        {
            if (it->second["root"].empty())
            {
                it->second["root"].push_back(server->getServerDir()["root"][0]);
            }
        }
        // default page when requesting a directory, copies root index by default
        for (std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = locationDirectives.begin(); it != locationDirectives.end(); ++it)
        {
            if (it->second["index"].empty())
            {
                it->second["index"].push_back(server->getServerDir()["index"][0]);
            }
        }
    }
    return 0;
}

int ConfigParser::extractDirectives(Server *server, TreeNode *node)
{
    if (node == NULL)
        return 1;
    std::vector<TreeNode *> children = node->getChildren();
    std::vector<TreeNode *>::iterator currentNode;

    for (currentNode = children.begin(); currentNode != children.end(); ++currentNode)
    {
        if ((*currentNode)->getChildren().size() == 0)
        {
            if (node->getDirective() == "server")
            {
                if (server->getServerDir()[(*currentNode)->getDirective()].size() > 0)
                {
                    Logger::error(this->getFileName(), "directive " + (*currentNode)->getDirective() + " already set");
                    return(1);
                }
                server->setServerDir((*currentNode)->getDirective(), (*currentNode)->getValue());
            }
            if (node->getDirective() == "location")
            {
                if (node->getValue().size() > 1)
                {
                    Logger::error(this->getFileName(), "location directive can't have more than one value");
                    return(1);
                }
                server->setLocationDir(node->getValue()[0], (*currentNode)->getDirective(), (*currentNode)->getValue());
            }
        }
        extractDirectives(server, *currentNode);
    }
    return 0;
}

int ConfigParser::checkMandatoryDirectives(Server *server)
{
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
    for (Mandatoryit = mandatoryServerDirectives.begin(); Mandatoryit != mandatoryServerDirectives.end(); ++Mandatoryit)
    {
        found = false;
        for (Serverit = serverDirectives.begin(); Serverit != serverDirectives.end(); ++Serverit)
        {
            if (Serverit->first == *Mandatoryit)
                found = true;
        }
        if (found == false)
        {
            Logger::error(this->getFileName(), "missing mandatory directive: " + *Mandatoryit);
            return 1;
        }
    }
    found = false;
    for (Mandatoryit = mandatoryCGIDirectives.begin(); Mandatoryit != mandatoryCGIDirectives.end(); ++Mandatoryit)
    {
        for (Locationit = locationDirectives.begin(); Locationit != locationDirectives.end(); ++Locationit)
        {

            if (Locationit->first == "/cgi-bin")
            {
                found = false;
                for (Serverit = Locationit->second.begin(); Serverit != Locationit->second.end(); ++Serverit)
                {
                    if (Serverit->first == *Mandatoryit)
                    {
                        found = true;
                    }
                }
                if (found == false)
                {
                    Logger::error(this->getFileName(), "missing mandatory directive: " + *Mandatoryit);
                    return 1;
                }
            }
        }
    }
    return 0;
}

void ConfigParser::setFileName(std::string file)
{
    this->fileName = file;
}

std::string ConfigParser::getFileName()
{
    return this->fileName;
}

std::vector<Server *> &ConfigParser::getTmpServer()
{
    return this->tmpServerArray;
}

void ConfigParser::setTmpServer(std::vector<Server *> server)
{
    this->tmpServerArray = server;
}

ConfigParser::ConfigParser()
{
    fnToParseDirectives["listen"] = &ConfigParser::parseListenValues;
    fnToParseDirectives["host"] = &ConfigParser::parseHostValues;
    fnToParseDirectives["server_name"] = &ConfigParser::parseServerNameValues;
    fnToParseDirectives["error_page"] = &ConfigParser::parseErrorPageValues;
    fnToParseDirectives["client_max_body_size"] = &ConfigParser::parseClientMaxBodyValues;
    fnToParseDirectives["root"] = &ConfigParser::parseRootValues;
    fnToParseDirectives["index"] = &ConfigParser::parseIndexValues;
    fnToParseDirectives["autoindex"] = &ConfigParser::parseAutoIndexValues;
    fnToParseDirectives["allow_methods"] = &ConfigParser::parseAllowMethodsValues;
    fnToParseDirectives["return"] = &ConfigParser::parseReturnValues;
    fnToParseDirectives["alias"] = &ConfigParser::parseAliasValues;
    fnToParseDirectives["cgi_ext"] = &ConfigParser::parseCgiExtValues;
    fnToParseDirectives["cgi_path"] = &ConfigParser::parseCGIPathValues;
    fnToParseDirectives["proxy_pass"] = &ConfigParser::parseProxyPassValues;

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
ConfigParser::~ConfigParser() {};