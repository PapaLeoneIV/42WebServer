#ifndef BOOTER_HPP
#define BOOTER_HPP

#include "Server.hpp"

typedef int SOCKET;

class Booter {
    public:

    Booter();
    ~Booter();

    void                       bootServer    (Server *server, const char *host, const char *port);
};


#endif