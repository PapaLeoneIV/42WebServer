#ifndef SERVER_HPP
#define SERVER_HPP
#define MAX_REQUEST_SIZE 3*1024*1024 //2MB


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



typedef int SOCKET;
typedef int ERROR;

class Server{

    public:    
    Server();
    ~Server();

    // GETTERS
    SOCKET       getServerSocket        (void);
    fd_set       getFdsSet              (void);
    addrinfo        &getHints               (void);
    addrinfo        *getBindAddrss          (void);
    std::string     &getHost                (void);
    std::string     &getServerName          (void);
    std::string     &getPort                (void);
    std::string     &getRoot                (void);
    std::string     &getIndex               (void);
    size_t       &getMaxRequestSize      (void);
    std::string     &getCwd                 (void);


    //SETTERS
    void    setServerSocket   (SOCKET server_socket);
    void    setFds            (fd_set fds);
    void    setHints          (addrinfo hints);
    void    setBindAddress    (addrinfo *bind_address);
    void    setHost           (std::string host);
    void    setServerName     (std::string server_name);
    void    setPort           (std::string port);
    void    setRoot           (std::string root);
    void    setIndex          (std::string index);
    void    setMaxRequestSize (size_t max_request_size);
    void    setCwd            (std::string cwd);
    private:

    //Server configuration taken from config file
    std::string                     _cwd;
    std::string                     _host;
    std::string                     _server_name;
    std::string                     _port;
    std::string                     _root;
    std::string                     _index;
    size_t                          _max_request_size;
    //Location configuration


    addrinfo                        _hints;
    SOCKET                          _server_socket;
    fd_set                          _fds;
    addrinfo                        *_bind_address;

};

#endif