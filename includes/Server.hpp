#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <stdexcept>

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


    //REQUEST PARSER

    //RESOURCE FINDER
    
    // Main function to handle incoming connections
    ERROR                       handleConnections       ();

    // Main loop where the server loops through all the list of clients and check if any of them has data to be read
    void                        serveClients            ();
    
    // Function to check if there is any new socket to be registred and accepted
    void                        registerNewConnections  ();

    // Monitor the sockets for an IO event 
    void                        monitorSocketIO         ();

    // Shut down all or part of the connection open on socket FD
    void                        closeClient             (SOCKET fd);

    // Send a response to the client
    ERROR                       sendResponse            (Response *response, SOCKET fd);
    
    // Get the client from the list of clients or creates a new one
    Client                      *getClient              (SOCKET socket);

    // Get the IP address of the client
    const char                  *getClientIP            (Client client);

    // Signal handler to stop the server
    void                        signalHandler           ();

    // GETTERS
    SOCKET                       getServerSocket        ();
    fd_set                       getFdsSet              ();
    addrinfo                    &getHints               ();
    addrinfo                    *getBindAddrss          ();

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

    //Object to parse data from the client
    Parser                       _parser;

    addrinfo                     _hints;
    SOCKET                       _server_socket;
    fd_set                       _fds;
    addrinfo                    *_bind_address;

};

#endif