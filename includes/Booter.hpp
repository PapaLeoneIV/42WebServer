#ifndef BOOTER_HPP
#define BOOTER_HPP

#include "Utils.hpp"
#include "Server.hpp"

class Booter {
    public:

    Booter();
    ~Booter();

    ERROR                       bootServer    (Server *server, const char *host, const char *port);
    
    ERROR                       GetAddrInfo   (Server *server, const char *host, const char *port);
    ERROR                       Socket        (Server *server);
    ERROR                       Bind          (Server *server);
    ERROR                       Fcntl         (Server *server);
    ERROR                       Listen        (Server *server);
};


#endif