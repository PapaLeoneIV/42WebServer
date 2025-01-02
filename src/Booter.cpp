#include "Booter.hpp"
#include "Server.hpp"

Booter::Booter(){};

Booter::~Booter(){};

SOCKET Booter::bootServer(Server *server, const char *host, const char *port)
{
    server->_keep_alive = true;

    ERROR error;

    std::cout << "Getting address info" << std::endl;
    if((error = resolveAddress(server, host, port))) { return error; }

    std::cout << "Creating socket" << std::endl;
    if((error = createSocket(server))) { return error; }

    std::cout << "Setting non-blocking" << std::endl;
    if((error = setNonBlockingFd(server))) { return error; }

    std::cout << "Binding" << std::endl;
    if((error = bindSocket(server))) { return error; }

    std::cout << "Listening" << std::endl;
    if((error = startListening(server))) { return error; }

    freeaddrinfo(server->getBindAddrss());

    return 0;
}

ERROR Booter::resolveAddress(Server *server, const char *host, const char *port) {
    struct addrinfo *addrinfo = NULL;
    ERROR error;

    error = getaddrinfo(host, port, &server->getHints(), &addrinfo);
    if (error != 0) {
        std::cerr << "Error resolving address: " << gai_strerror(error) << std::endl;
        return error;
    }

    server->setBindAddress(addrinfo);

    return 0;
}



ERROR Booter::createSocket(Server *server)
{   
    SOCKET socket_fd;

    socket_fd = socket((server->getHints()).ai_family, (server->getHints()).ai_socktype, (server->getHints()).ai_protocol);
    if (socket_fd == -1){
        std::cerr << "Error: socket creation failed" << std::endl;
        //TODO throw exception
        return 1;
    }

    server->setServerSocket(socket_fd); 

    return 0;
}

ERROR Booter::setNonBlockingFd(Server *server)
{
    ERROR error;
    int flags;

    if((flags = fcntl(server->getServerSocket(), F_GETFL, 0)) == -1)
        flags |= O_NONBLOCK;  
    if((error = fcntl(server->getServerSocket(), F_SETFL, flags))){
        std::cerr << "Error: setting socket to non-blocking failed" << std::endl;
        //TODO throw exception
        return error;
    }
    return 0;
}

ERROR Booter::bindSocket(Server *server)
{
    if(bind(server->getServerSocket(), server->getBindAddrss()->ai_addr, server->getBindAddrss()->ai_addrlen) == -1){
        std::cerr << "Error: bind failed" << std::endl;
        //TODO throw exception
        return 1;
    }
    return 0;
}

ERROR Booter::startListening(Server *server)
{
    if(listen(server->getServerSocket(), 10) == -1){
        std::cerr << "Error: listen failed" << std::endl;
        //TODO throw exception
        return 1;
    }
    return 0;
}

