#include "Server.hpp"


void Server::closeConnection(SOCKET fd) {
    shutdown(fd, SHUT_RDWR);
    close(fd);
}


const char *Server::getClientIP(Client client)
{
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client.getAddr(), client.getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return address_info;
}


//GETTERS
SOCKET                       Server::getServerSocket() { return this->_server_socket;}
fd_set                       Server::getFdsSet() { return this->_fds;}
addrinfo                     &Server::getHints() { return this->_hints;}
addrinfo                     *Server::getBindAddrss() { return this->_bind_address;}

//SETTERS
void                         Server::setServerSocket(SOCKET server_socket){this->_server_socket = server_socket;};
void                         Server::setFds(fd_set fds){this->_fds = fds;};
void                         Server::setHints(addrinfo hints){this->_hints = hints;};
void                         Server::setBindAddress(addrinfo *bind_address){this->_bind_address = bind_address;};

Server::Server(): _clients(),_keep_alive(true), _hints(), _server_socket(-1), _bind_address(0) {

    this->_hints.ai_family = AF_INET;      
    this->_hints.ai_socktype = SOCK_STREAM;
    this->_hints.ai_flags = AI_PASSIVE;

}

Server::~Server(){}