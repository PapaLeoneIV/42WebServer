#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>


typedef int SOCKET;


#define MAX_REQUEST_SIZE 1024


class Client{
    public:

    socklen_t address_length;
    sockaddr_storage address;
    SOCKET socket;
    char request[MAX_REQUEST_SIZE + 1];
    int received;

    Client(){
        memset(&this->address, 0, sizeof(this->address));
        this->address_length = sizeof(this->address);
        this->received = 0;
    }; 
    ~Client(){};

};

#endif