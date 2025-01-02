#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include "Request.hpp"
#include "Utils.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

class Client{

    public:

    Client();
    Client(Client& other);
    ~Client();

    sockaddr_storage        &getAddr();
    socklen_t               &getAddrLen();
    SOCKET                  &getSocketFd();
    Request                 *get_Request();
    char                    *getRequest();
    int                     &getRecvBytes();

    void                    setAddr(sockaddr_storage addr); 
    void                    setAddrLen(socklen_t len);
    void                    setSocketFd(SOCKET fd);
    void                    setRequestData(char * requestData);
    void                    set_Request(Request *request);
    void                    set_recv_bytes(int bytes);
    
                            
    private:


    Request                 *_Request;
    socklen_t               _address_length;
    sockaddr_storage        _address;
    SOCKET                  _socket;
    char                    _requestData[MAX_REQUEST_SIZE + 1];
    int                     _received;
};

#endif