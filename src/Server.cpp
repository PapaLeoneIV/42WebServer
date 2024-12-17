#include "Server.hpp"
#include "Client.hpp"
#include <iostream>



#define TIMEOUT_SEC 5

ERROR Server::handle_connections(){
    
    ERROR error;

    this->monitor_socket_fds();
    
    if((error = this->monitor_new_connections(this->fds))) { return error; }

    std::vector<Client>::iterator client = this->clients.begin();
    while (client != this->clients.end())
    {
        if(FD_ISSET(client->socket, &this->fds))
        {
            if(MAX_REQUEST_SIZE == client->received)
            {
                std::cerr << "Error: request too long: closing connection" << std::endl;
                //TODO send 400 BAD REQUEST
                close(client->socket);
                client = this->clients.erase(client);
                continue;
            }
            int bytes_received = recv(client->socket, client->request + client->received, MAX_REQUEST_SIZE - client->received, 0);
            if (bytes_received < 1){
                std::cerr << "Error: recv failed: closing connection" << std::endl;
                //TODO send 500 INTERNAL SERVER ERROR
                close(client->socket);
                client = this->clients.erase(client);
                continue;
            }
            client->received += bytes_received;
            if(client->received > 2 && client->request[client->received - 2] == '\r' && client->request[client->received - 1] == '\n')
            {
                std::cout << "Received: " << client->received << " bytes" << std::endl;
                std::cout << "Request: " << client->request << std::endl;
                //TODO process request
                client->received = 0;
            }
        }
        ++client;
    } // CLIENT LOOP
    return 0;
}


ERROR Server::monitor_new_connections(fd_set fds)
{
    //controlla se qualcuno vuole connettersi(controllando che il server sia pronto a leggere)
    if(FD_ISSET(this->server_socket, &fds))
    {
        Client client = this->get_client(-1);
        client.socket = accept(this->server_socket, (sockaddr*)&client.address, &client.address_length);
        if (client.socket < 0)
        {
            std::cerr << "Error: accept failed" << std::endl;
            //TODO throw exception
            return 1;
        }
        this->clients.push_back(client);
        std::cout << "New connection from " << this->get_clientIP(client) << std::endl;
    }
    return 0;
}

void Server::monitor_socket_fds()
{
    FD_ZERO(&this->fds);
    FD_SET(this->server_socket, &this->fds);
    
    SOCKET max_socket = this->server_socket;
    
    std::vector<Client>::iterator it;
    
    for(it = this->clients.begin(); it != this->clients.end(); it++)
    {
        FD_SET(it->socket, &this->fds);
        if(it->socket > max_socket)
        {
            max_socket = it->socket;
        }
    }

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    if(select(max_socket + 1, &this->fds, 0, 0, &timeout) < 0)
    {
        throw std::runtime_error("Error: select failed");
    }
}

Client Server::get_client(SOCKET fd)
{
    std::vector<Client>::iterator it;
    std::cout << "Checking if client is already in the list" << std::endl;
    for(it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if(it->socket == fd) 
            return *it;
    }

    std::cout << "Client not found, creating a new one" << std::endl;
    
    Client new_client;
    
    new_client.socket = fd;
    new_client.address_length = sizeof(new_client.address);
    
    return new_client;
}

const char *Server::get_clientIP(Client client)
{
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client.address, client.address_length, address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return address_info;
}

//BOOT
SOCKET Server::boot_server(const char *host, const char *port)
{
    this->keep_alive = true;

    ERROR error;

    std::cout << "Getting address info" << std::endl;
    if((error = resolve_address(host, port))) { return error; }

    std::cout << "Creating socket" << std::endl;
    if((error = create_socket())) { return error; }

    std::cout << "Binding" << std::endl;
    if((error = bind_socket())) { return error; }

    std::cout << "Listening" << std::endl;
    if((error = start_listening())) { return error; }

    freeaddrinfo(this->bind_address);

    return 0;
}

ERROR Server::resolve_address(const char *host, const char *port)
{

    memset(&this->hints, 0, sizeof(this->hints));
    this->hints.ai_family = AF_INET;
    this->hints.ai_socktype = SOCK_STREAM;
    this->hints.ai_flags = AI_PASSIVE;

    ERROR error;

    error = getaddrinfo(host, port, &this->hints, &this->bind_address);
    if(error)
    {
        std::cerr << gai_strerror(error) << std::endl;
        return error;
    }
    return 0;
}

ERROR Server::create_socket()
{
    this->server_socket = socket(this->hints.ai_family, this->hints.ai_socktype, this->hints.ai_protocol);
    if (server_socket == -1)
    {
        std::cerr << "Error: socket creation failed" << std::endl;
        //TODO throw exception
        return 1;
    }
    return 0;
}

ERROR Server::bind_socket()
{
    if(bind(this->server_socket, bind_address->ai_addr, bind_address->ai_addrlen) == -1)
    {
        std::cerr << "Error: bind failed" << std::endl;
        //TODO throw exception
        return 1;
    }
    return 0;
}

ERROR Server::start_listening()
{
    if(listen(this->server_socket, 10) == -1)
    {
        std::cerr << "Error: listen failed" << std::endl;
        //TODO throw exception
        return 1;
    }
    return 0;
}

// GETTERS
SOCKET Server::get_server_socket()
{
    return this->server_socket;
}





Server::Server()
{
    this->clients = std::vector<Client>();
    this->bind_address = 0;
    this->server_socket = -1;
    this->keep_alive = true;
}

Server::~Server()
{
}