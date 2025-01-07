#ifndef BOOTER_HPP
#define BOOTER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string.h>

#include "Server.hpp"

class Booter {
    public:

    Booter();
    ~Booter();

    ERROR                       bootServer          (Server *server, const char *host, const char *port);
    
    ERROR                       resolveAddress      (Server *server, const char *host, const char *port);
    ERROR                       setNonBlockingFd    (Server *server);
    ERROR                       startListening      (Server *server);
    ERROR                       createSocket        (Server *server);
    ERROR                       bindSocket          (Server *server);
};


#endif