#ifndef PARSER_HPP
#define PARSER_HPP

#include "Request.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include "Server.hpp"

class Parser{
    public:

    Request *decompose  (char *data);

    ERROR    parse   (Request *request, Client *client);

    void    validateResource    (Client *client, Server *server);

    std::string readFile    (std::string filePath, Response *response);

    void   checkAccessability  (std::string filePath, Response *response);

    Parser();
    ~Parser();

    private:
    std::set<std::string>                    _implemnted_methods;
    std::set<std::string>                    _allowd_methods;
    std::set<std::string>                    _allowd_versions;
    std::set<std::string>                    _allowd_headers;
};

#endif