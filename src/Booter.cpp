#include "Booter.hpp"
#include "Server.hpp"

Booter::Booter(){};

Booter::~Booter(){};

ERROR Booter::bootServer(Server *server, const char *host, const char *port)
{
    server->_keep_alive = true;

    ERROR error;

    std::cout << "Getting address info" << std::endl;
    if((error = GetAddrInfo(server, host, port))) { return error; }

    std::cout << "Creating socket" << std::endl;
    if((error = Socket(server))) { return error; }

    std::cout << "Setting non-blocking" << std::endl;
    if((error = Fcntl(server))) { return error; }

    std::cout << "Binding" << std::endl;
    if((error = Bind(server))) { return error; }

    std::cout << "Listening on port: " << port << std::endl;
    if((error = Listen(server))) { return error; }

    freeaddrinfo(server->getBindAddrss());

    return SUCCESS;
}

ERROR Booter::GetAddrInfo(Server *server, const char *host, const char *port) {
    struct addrinfo *addrinfo = NULL;

    if (getaddrinfo(host, port, &server->getHints(), &addrinfo)) {
        return ERR_RESOLVE_ADDR;
    }

    server->setBindAddress(addrinfo);

    return SUCCESS;
}

ERROR Booter::Socket(Server *server)
{   
    SOCKET socket_fd;

    socket_fd = socket((server->getHints()).ai_family, (server->getHints()).ai_socktype, (server->getHints()).ai_protocol);

    if (socket_fd == -1){
        return ERR_SOCK_CREATION;
    }

    server->setServerSocket(socket_fd); 

    return SUCCESS;
}

ERROR Booter::Fcntl(Server *server)
{
    ERROR error;
    int flags;

    if((flags = fcntl(server->getServerSocket(), F_GETFL, 0)) == -1)
        flags |= O_NONBLOCK;  
    if((error = fcntl(server->getServerSocket(), F_SETFL, flags))){
        return ERR_SOCKET_NBLOCK;
    }
    return SUCCESS;
}

ERROR Booter::Bind(Server *server)
{
    if(bind(server->getServerSocket(), server->getBindAddrss()->ai_addr, server->getBindAddrss()->ai_addrlen) == -1){
        return ERR_BIND;
    }
    return SUCCESS;
}

ERROR Booter::Listen(Server *server)
{
    if(listen(server->getServerSocket(), 10) == -1){
        return ERR_LISTEN;
    }
    return SUCCESS;
}

