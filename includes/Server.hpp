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

    // Shut down all or part of the connection open on socket FD
    void                        closeConnection         (SOCKET fd);

    // Get the client from the list of clients or creates a new one
    Client                      *getClient              (SOCKET socket);

    // Get the IP address of the client
    const char                  *getClientIP            (Client client);

    // GETTERS
    SOCKET                       getServerSocket        (void);
    fd_set                       getFdsSet              (void);
    addrinfo                    &getHints               (void);
    addrinfo                    *getBindAddrss          (void);

    //SETTERS
    void                         setServerSocket        (SOCKET server_socket);
    void                         setFds                 (fd_set fds);
    void                         setHints               (addrinfo hints);
    void                         setBindAddress         (addrinfo *bind_address);

    //Vector of registred clients
    std::vector<Client*>        _clients;
    
    //Flag to keep the server alive
    bool                        _keep_alive;

    private:

    addrinfo                     _hints;
    SOCKET                       _server_socket;
    fd_set                       _fds;
    addrinfo                    *_bind_address;

};

#endif