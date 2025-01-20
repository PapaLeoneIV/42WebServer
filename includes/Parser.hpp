#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP


#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <algorithm>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <set>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"

typedef int SOCKET;
typedef int ERROR;

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