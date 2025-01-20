#ifndef PARSER_HPP
#define PARSER_HPP

#include "Request.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include "Server.hpp"

class Parser{
    public:

    Request     *decompose  (std::string headerData, std::string bodyData, Client *client);

    ERROR       parse   (Request *request, Client *client);

    void        validateResource    (Client *client, Server *server);

    std::string readFile    (std::string filePath, Response *response);

    int         checkResource  (std::string filePath, Response *response);
    void        parseMultipart(Request *request, std::istringstream &iss, std::string boundary);

    void        decomposeFirstLine( Request *request, std::string firstLine);
    void        decomposeHeaders(Request *request, std::string headerData);
    void        decomposeBody(Request *tmpRequest, std::string bodyData);
    void        parseChunked(Request *request, std::istringstream &bodyStream);
    Parser();
    ~Parser();

    private:
    std::set<std::string>                    _implemnted_methods;
    std::set<std::string>                    _allowd_methods;
    std::set<std::string>                    _allowd_versions;
    std::set<std::string>                    _allowd_headers;
};

#endif