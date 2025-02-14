
#include "Booter.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ServerManager.hpp"
#include "Utils.hpp"


/**
 * Event loop, il programma tramite la funzione bloccante select(), monitora
 * i sockets nelle varie pool (read/write). Se uno degl fd switcha stato(I/O) select()
 * esce e la Request viene gestita nel loop interno.
*/
void ServerManager::eventLoop()
{
    while(420){
        int fds_changed = 0;
        
        //bisogna resettare gli fd ad ogni nuovo ciclo
        FD_ZERO(&this->_readPool);
        FD_ZERO(&this->_writePool);
        
        this->initFdSets();

        //select() resetta 'timeout' ad ogni ciclo, necessati dunque di essere re inizializata
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        //la gestione degl fd e' semplificata tramite l utilizzo di una pool generica. I nuovi fd
        //vengono aggiunti in masterpool e copiati nei rispetti bit array
        this->_readPool = this->_masterPool;
        this->_writePool = this->_masterPool;

        if ((fds_changed = select(this->_maxSocket + 1, &this->_readPool, &this->_writePool, 0, &timeout)) < 0)
            throw std::runtime_error(ErrToStr(ERR_SELECT));
        
        if(fds_changed == 0) continue;

        for (SOCKET fd = 0; fd <= this->_maxSocket + 1; ++fd){

            if(FD_ISSET(fd, &this->_readPool) && this->_servers_map.count(fd) > 0){
                this->registerNewConnections(fd, this->_servers_map[fd]);
            }
            if(FD_ISSET(fd, &this->_readPool) && this->_clients_map.count(fd) > 0){
                this->processRequest(this->_clients_map[fd]);
            }
            if(FD_ISSET(fd, &this->_writePool) && this->_clients_map.count(fd) > 0){
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

    std::cout << "["<< new_socket <<  "] INFO: New connection from " << this->getClientIP(client) << std::endl;
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
    int bytesRecv = recv(client->getSocketFd(), buffer, sizeof(buffer), 0); //O_NONBLOCK

    if(bytesRecv == -1){
        this->removeClient(client->getSocketFd());
        return;
    }
    
    if(bytesRecv == 0){
        this->removeClient(client->getSocketFd());
        return;
    }
    
    if(bytesRecv > 0){
        client->getRequest()->consume(buffer);
    }



    if(client->getRequest()->state == StateParsingComplete /*TODO: prepare error response if there is an error in consume() */){
        
        // TODO: based on the value from the config file, we need to decide if it is a valid request
        // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/16
        // eg: if the method is in the allowed methods(direttiva del config-file)
        // eg: if proxy_pass is set, i think, not sure, we need to make a send() with the request to the proxy_pass server

        // TODO: based on the URL (credo), we need to decide if we need to pass the request to CGI
        // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/15
        
        parser.validateResource(client, client->getServer());
        this->addToSet(client->getSocketFd(), &this->_masterPool);
    }
}

void ServerManager::sendResponse(SOCKET fd, Client *client)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    //safety checks perche in realta sono scarso e senza questi e' tutto buggoso
    if(!request || !response || client->getRequest()->state != StateParsingComplete){
        return;
    }

    // TODO: maybe, it will be better to move the response generation into a separate component 
    // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/14
    response->setHeaders("Host", "localhost");
    if(!response->getBody().empty()){
        response->setHeaders("Content-Type", getContentType(request->getUrl(), response->getStatus()));
        response->setHeaders("Content-Length", intToStr(response->getBody().size()));
    }
    
    if(request->getHeaders()["connection"] == "close")
        response->setHeaders("Connection", "close");

    response->prepareResponse();

    int bytes_sent = send(fd, response->getResponse().c_str(), response->getResponse().size(), 0);

    if (bytes_sent == -1){
        std::cerr << "Error: send failed: closing connection" << std::endl;
        this->removeClient(fd);
        return;
    }

    // TODO: check if we need to close the connection or if we can keep the client fd open for next request 
    // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/13
    if(request->getHeaders()["connection"] == "close")
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
    //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/12
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

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

}

ServerManager::~ServerManager(){}

