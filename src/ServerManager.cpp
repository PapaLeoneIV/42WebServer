
#include "Booter.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ServerManager.hpp"
#include "Utils.hpp"

void ServerManager::mainLoop()
{
    this->initFdSets();
    while(420){
        int fds_changed = 0;
        
        FD_ZERO(&this->_readPool);
        FD_ZERO(&this->_writePool);
        
        this->_readPool = this->_masterPool;
        this->_writePool = this->_masterPool;

        if ((fds_changed = select(this->_maxSocket + 1, &this->_readPool, &this->_writePool, 0, NULL)) < 0)
            throw std::runtime_error(ErrToStr(ERR_SELECT));
        
        
        if(fds_changed == 0) continue;
        
        std::cout << "Select switched n fds [ " << fds_changed << " ]" << std::endl;
        for (SOCKET fd = 0; fd <= this->_maxSocket + 1; ++fd){

            if(FD_ISSET(fd, &this->_readPool) && this->_servers_map.count(fd) > 0){
                std::cout << "Handling new connection" << std::endl;
                this->registerNewConnections(fd, this->_servers_map[fd]);
            }
            if(FD_ISSET(fd, &this->_readPool) && this->_clients_map.count(fd) > 0){
                std::cout << "Handling Request process" << std::endl;
                this->processRequest(this->_clients_map[fd]);

            }
            if(FD_ISSET(fd, &this->_writePool) && this->_clients_map.count(fd) > 0){
                std::cout << "Sending Response" << std::endl;
                this->sendResponse(fd, this->_clients_map[fd]);
            }
        }
    }
}


void ServerManager::registerNewConnections(SOCKET serverFd, Server *server)
{
    //creo un nuovo client se non esiste gia
    Client* client = this->getClient(-1);
    if (client == NULL) { //allocazione andata male
        return;
    }
    
    //accetto la connessione
    SOCKET new_socket = accept(serverFd, (sockaddr*)&client->getAddr(), &client->getAddrLen()); 
    if (new_socket < 0){
        delete client;
        return;
    }

    //setto il socket come non bloccante
    if(fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0){
        std::cout << "Error: fcntl failed" << std::endl;
        delete client;
        close(new_socket);
        return;
    }

    std::cout << "[ "<< new_socket <<  " ] New connection from " << this->getClientIP(client) << std::endl;
    //setto il nuovo socket al client
    client->setSocketFd(new_socket);
    //aggiungo il socket al pool di socket da monitorare
    this->addToSet(new_socket, &this->_masterPool);
    //aggiungo un riferimento al server all interno del client
    client->setServer(server);
    
    this->_clients_map[new_socket] = client;

}

#define BUFFER_SIZE 4*1024 //4KB

void ServerManager::processRequest(Client *client)
{
    Parser parser;

    char buffer[BUFFER_SIZE];
    std::cout << "client socket : " << client->getSocketFd() << std::endl; 
    int bytesRecv = recv(client->getSocketFd(), buffer, sizeof(buffer), 0);

    std::cout << "Buffer of request: " << buffer << std::endl;
    // TODO: not here to stay
    if(errno == EWOULDBLOCK || errno == EAGAIN){
        std::cerr << "Error: failed with EWOULDBLOCK || EAGAIN" << std::endl;
        return;
    }
    if(bytesRecv == -1){
        std::cout << errno << std::endl;
        std::cerr << "Error: recv failed with error: closing connection" << std::endl;
        this->removeClient(client->getSocketFd());
        return;
    }
    
    if(bytesRecv == 0){
        std::cerr << "Error: recv failed: closing connection" << std::endl;
        this->removeClient(client->getSocketFd());
        return;
    }
    
    if(bytesRecv > 0){
        std::cout << "bytes received" << bytesRecv << std::endl;
        client->getRequest()->consume(buffer);
    }


    //TODO handle better what i have refactored

    //client->getRequest()->print_Request();
    if(client->getRequest()->state == StateParsingComplete ){
        parser.validateResource(client, client->getServer());
    }
    //     if(client->getRequest()->has_body)
    //         client->getRequest()->setBody(client->getRequest()->body);
        
    //     if(client->getRequest()->method == "GET" || client->getRequest()->method == "DELETE"){
    //         parser.validateResource(client, client->getServer());
    //     } else if (client->getRequest()->method == "POST"){
    //          //TODO: implement POST
    //     } else {
    //        client->getResponse()->setStatusCode(405);
    //     }
    // }


}

void ServerManager::sendResponse(SOCKET fd, Client *client)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response){
        return;
    }
    
    response->setHeaders("Host", "localhost");
    //std::cout << "Response body:" << response->getBody() << std::endl; 
    if(!response->getBody().empty()){
        response->setHeaders("Content-Type", getContentType(request->getUrl(), response->getStatus()));
        response->setHeaders("Content-Length", intToStr(response->getBody().size()));
    }
    // } else {
    //     response->setHeaders("Content-Length", "0");
    // }
    
    if(request->getHeaders()["connection"] == "close")
        response->setHeaders("Connection", "close");

    response->prepareResponse();

    //response->print();

    int bytes_sent = send(fd, response->getResponse().c_str(), response->getResponse().size(), 0);

    if (bytes_sent == -1){
        std::cerr << "Error: send failed: closing connection" << std::endl;
        this->removeClient(fd);
        return;
    }

    //if(request->getHeaders()["connection"] == "close")
    this->removeClient(fd);
    
    return; 
}

void ServerManager::initFdSets()
{
    for (std::map<SOCKET, Server*>::iterator server_it = this->_servers_map.begin(); server_it != this->_servers_map.end(); ++server_it){
        FD_SET(server_it->first, &this->_masterPool);
        if(server_it->first > this->_maxSocket){
            this->_maxSocket = server_it->first;
        }
    }
    //TODO: client non penso ce ne possono essere in questo momento
    for (std::map<SOCKET, Client*>::iterator clientIt = this->_clients_map.begin(); clientIt != this->_clients_map.end(); ++clientIt){
        FD_SET(clientIt->first, &this->_masterPool);
        this->_maxSocket = std::max(this->_maxSocket, clientIt->first);
    }

}


ServerManager::ServerManager()
{
    this->_maxSocket = 0;
    
    FD_ZERO(&this->_readPool);
    FD_ZERO(&this->_writePool);
    FD_ZERO(&this->_masterPool);

    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;

}

ServerManager::~ServerManager(){}

