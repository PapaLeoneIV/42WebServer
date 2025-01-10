#ifndef PARSER_HPP
#define PARSER_HPP

#include "Request.hpp"
#include "Client.hpp"
#include "Utils.hpp"



class Parser{
    public:

    Request *decompose                       (char *data);

    void    parse                            (Request *request, Client *client);

    Parser();
    ~Parser();

    private:
    std::set<std::string>                    _implemnted_methods;
    std::set<std::string>                    _allowd_methods;
    std::set<std::string>                    _allowd_versions;
    std::set<std::string>                    _allowd_headers;
};

#endif