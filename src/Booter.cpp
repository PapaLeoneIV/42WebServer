#include "Booter.hpp"
#include "Server.hpp"
#include "Utils.hpp"

Booter::Booter(){};

Booter::~Booter(){};

void Booter::bootServer(Server *server, const char *host, const char *port){
    struct addrinfo *addrinfo = NULL;

    if (getaddrinfo(host, port, &server->getHints(), &addrinfo)) {
        exit(ERR_RESOLVE_ADDR);
    }

    server->setBindAddress(addrinfo);

    SOCKET socket_fd;
    int optval = 1;
    
    socket_fd = socket((server->getHints()).ai_family, (server->getHints()).ai_socktype, (server->getHints()).ai_protocol);

    if (socket_fd == -1)
        exit(ERR_SOCK_CREATION);

    server->setServerSocket(socket_fd); 
    
    if (setsockopt(server->getServerSocket(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        throw std::runtime_error(ErrToStr(1));

    if(fcntl(server->getServerSocket(), F_SETFL, O_NONBLOCK) < 0)
        exit(ERR_FCNTL);

    if(bind(server->getServerSocket(), server->getBindAddrss()->ai_addr, server->getBindAddrss()->ai_addrlen) == -1)
        exit(ERR_BIND);

    if(listen(server->getServerSocket(), 10) == -1)
        exit(ERR_LISTEN);

    freeaddrinfo(server->getBindAddrss());

}
