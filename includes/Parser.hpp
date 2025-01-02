#ifndef PARSER_HPP
#define PARSER_HPP

#include "Request.hpp"
#include "Client.hpp"

#include "Utils.hpp"

#include <vector>
#include <iostream>
#include <sstream>

class Parser{
    public:

    Request                         *parse              (std::vector<Client*>::iterator client);

    ERROR                           validate            (Request *request);

    ERROR                           checkMethod         (std::string method);
    ERROR                           checkUrl            (std::string url);
    ERROR                           checkVersion        (std::string version);
    ERROR                           checkHeaders        (std::map<std::string, std::string>  headers);
    ERROR                           checkBody           (std::string body, Request *request);

    ERROR                           checkRecvBytes      (int recv_bytes);
    ERROR                           checkMaxRecvBytes   (int recv_bytes);



    Parser();
    ~Parser();

    private:
    
    std::vector<std::string>            _allowd_methods;
    std::vector<std::string>            _allowd_versions;
    std::vector<std::string>            _mandatory_headers;
    std::vector<std::string>            _allowd_headers;
};

#endif