#pragma once

#ifndef BOOTER_HPP
#define BOOTER_HPP


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

#include "Server.hpp"


typedef int SOCKET;
typedef int ERROR;

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