#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <string>
#include <set>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"

typedef int SOCKET;
typedef int ERROR;

class Parser{
    public:

    Request *extract  (std::string headerData, std::string bodyData, Client *client);

    ERROR   extractFirstLine(Request *request, Response *response, std::string firstLine);
    
    void    extractHeaders(Request *request, std::istringstream &headerStream);
    
    ERROR   extractBody(Request *request, std::istringstream &bodyStream);
    
    void    parseMultipart(Request *request, std::istringstream &iss, std::string boundary);
    
    void    validateResource    (Client *client, Server *server);

    std::string readFile    (std::string filePath, Response *response);

    int     checkResource  (std::string filePath, Response *response);
    
    Parser();
    ~Parser();

    private:
    std::set<std::string>                    _implemnted_methods;
    std::set<std::string>                    _allowd_methods;
    std::set<std::string>                    _allowd_versions;
    std::set<std::string>                    _allowd_headers;
};

#endif