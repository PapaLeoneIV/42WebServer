#pragma once
#ifndef CLIENTS_HPP
#define CLIENTS_HPP


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
#include "Request.hpp"
#include "Response.hpp"


typedef int SOCKET;
typedef int ERROR;

class Client{

    public:

    Client();
    Client(Client& other);
    Client(SOCKET fd);

    ~Client();

    Response                *getResponse        (void);
    Request                 *getRequest         (void);
    
    sockaddr_storage        &getAddr            (void);
    socklen_t               &getAddrLen         (void);
    SOCKET                  &getSocketFd        (void);
    std::string             getRequestData     (void);
    int                     &getRecvBytes       (void);
    Server*                 getServer           (void); 
    std::string             getHeadersData     (void);
    std::string             getBodyData        (void);
    int                     getContentLength    (void);

    void                    set_Request         (Request *request);
    void                    set_Response        (Response *response);

    void                    setAddr             (sockaddr_storage addr); 
    void                    setAddrLen          (socklen_t len);
    void                    setSocketFd         (SOCKET fd);
    void                    setRequestData      (std::string requestData);
    void                    setRecvData         (int bytes);
    void                    setServer           (Server *server);
    void                    setHeadersData      (std::string headersData);
    void                    setBodyData         (std::string bodyData);
    void                    setContentLength         (int content_length); 
                            
    private:


    Request                 *_Request;
    Response                *_Response;

    Server                  *_server;
    
    sockaddr_storage        _address;
    socklen_t               _address_length;
    SOCKET                  _socket;
    std::string             _requestData;
    std::string             _headersData;
    std::string             _bodyData; 
    int                     _content_length;
    int                     _received;
};

#endif