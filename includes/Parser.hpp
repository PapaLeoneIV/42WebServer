#ifndef PARSER_HPP
#define PARSER_HPP

#include <set>
#include <string>
#include <sys/unistd.h>
#include <vector>

class Server;
class Client;
class Response;

typedef int SOCKET;
typedef int ERROR;

class Request;

class Parser{
    public:
    
    //void    parseMultipart(Request *request, std::istringstream &iss, std::string boundary);
    
    void    validateResource    (Client *client, Server *server);

    std::string readFile    (std::string filePath, Response *response);

    int     checkResource  (std::string filePath, Response *response, int accessMode = R_OK);
    
    int     deleteResource  (std::string filePath, Response *response, bool useDetailedResponse = true);

    std::string     extractQueryParams(const std::string &url, const std::string  &paramName, const std::string &defaultValue="", const std::vector<std::string> &validValues = std::vector<std::string>());

    bool        isQueryParamValid(const std::string &url, const std::string &paramName, bool defaultValue = false);
    
    Parser();
    ~Parser();

    private:
    std::set<std::string>                    _implemnted_methods;
    std::set<std::string>                    _allowd_methods;
    std::set<std::string>                    _allowd_versions;
    std::set<std::string>                    _allowd_headers;
};

#endif
