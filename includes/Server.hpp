#ifndef SERVER_HPP
#define SERVER_HPP



#include "Utils.hpp"
#include "Client.hpp"
#include "Parser.hpp"
#include "Response.hpp"



/**
 * 
 * 
 * 
 */
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
    size_t      &getMaxRequestSize      (void);


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
    
    private:

    //Server configuration
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