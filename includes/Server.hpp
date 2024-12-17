#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>

#include "Client.hpp"


typedef int SOCKET;
typedef int ERROR;


class Server{

    public:

    std::vector<Client>clients;
    
    bool keep_alive;
    
    //BOOT
    SOCKET                      boot_server(const char *host, const char *port);
    ERROR                       resolve_address(const char *host, const char *port);
    ERROR                       create_socket();
    ERROR                       bind_socket();
    ERROR                       start_listening();
    const char                  *get_clientIP(Client client);
    
    //MONITOR
    void                        monitor_socket_fds();   
    ERROR                       monitor_new_connections(fd_set fds);
    ERROR                       handle_connections();

    Client                      get_client(SOCKET socket);
    
    // GETTERS
    SOCKET                       get_server_socket();

                                Server();
                                ~Server();

    private:

    SOCKET                       server_socket;
    fd_set                       fds;
    addrinfo                     hints;
    addrinfo *                   bind_address;

};

#endif