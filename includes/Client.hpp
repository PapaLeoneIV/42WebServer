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

    sockaddr_storage        &get_addr();
    socklen_t               &get_addr_len();
    SOCKET                  &get_socket_fd();
    char                    *get_request();
    int                     &get_recv_bytes();
                            

    void                    set_addr(sockaddr_storage addr); 
    void                    set_addr_len(socklen_t len);
    void                    set_socket_fd(SOCKET fd);
    void                    set_request(char * request);
    void                    set_recv_bytes(int bytes);

    void                    add_recv_bytes(int bytes);
    
                            Client();
                            ~Client();
    private: 
    socklen_t               address_length;
    sockaddr_storage        address;
    SOCKET                  socket;
    char                    request[MAX_REQUEST_SIZE + 1];
    int                     received;
};

#endif