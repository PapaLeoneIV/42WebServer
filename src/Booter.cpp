#include "Booter.hpp"
#include "Server.hpp"
#include "Utils.hpp"
Booter::Booter(){};

Booter::~Booter(){};

ERROR Booter::bootServer(Server *server, const char *host, const char *port)
{
    ERROR error;

    if((error = GetAddrInfo(server, host, port))) { return error; }

    if((error = Socket(server))) { return error; }

    if((error = Fcntl(server))) { return error; }

    if((error = Bind(server))) { return error; }

    if((error = Listen(server))) { return error; }

    freeaddrinfo(server->getBindAddrss());

    std::cout << "[ " <<  server->getServerSocket() << " ]" << " Listening on "<< host << ":" << port <<   std::endl;


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
    int optval = 1;
    socket_fd = socket((server->getHints()).ai_family, (server->getHints()).ai_socktype, (server->getHints()).ai_protocol);

    if (socket_fd == -1){
        return ERR_SOCK_CREATION;
    }

   
    server->setServerSocket(socket_fd); 
    if (setsockopt(server->getServerSocket(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        throw std::runtime_error(ErrToStr(1));
	}
    return SUCCESS;
}

ERROR Booter::Fcntl(Server *server)
{
    if(fcntl(server->getServerSocket(), F_SETFL, O_NONBLOCK) < 0)
        return ERR_FCNTL;
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

