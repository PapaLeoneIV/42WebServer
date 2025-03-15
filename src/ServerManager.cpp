#include "../includes/Booter.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/ServerManager.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#include <cstdlib>


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

        // Controllo i timeout delle connessioni persistenti
        time_t currentTime = time(NULL);
        this->handleClientTimeout(currentTime);

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
        Logger::error("ServerManager", "Error: fcntl failed");
        delete client;
        close(new_socket);
        return;
    } 

    Logger::info("New connection from " + this->getClientIP(client) + " [" + intToStr(new_socket) + "]");
    //setto il nuovo socket al client
    client->setSocketFd(new_socket);
    //aggiungo il socket al pool di socket da monitorare
    this->addToSet(new_socket, &this->_masterPool);
    //aggiungo un riferimento al server all interno del client
    client->setServer(server);
    //inizializzo l'ultimo accesso
    client->updateLastActivity();
    
    this->_clients_map[new_socket] = client;

}

#define BUFFER_SIZE 4*1024 //4KB


void ServerManager::processRequest(Client *client)
{
    Parser parser;
    SOCKET fd = client->getSocketFd();
    
    // this->debugPools("Prima di processRequest", fd);

    client->updateLastActivity();

    char buffer[BUFFER_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));

    int bytesRecv = recv(fd, buffer, BUFFER_SIZE, 0); //O_NONBLOCK

    if(bytesRecv == -1){
        Logger::error("ServerManager", "Error in recv(): " + std::string(strerror(errno)) + " [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
        return;
    }
    
    if(bytesRecv == 0){
        Logger::info("Client closed connection [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
        return;
    }
    
    if(bytesRecv > 0){
        Logger::info("Received " + intToStr(bytesRecv) + " bytes [" + intToStr(fd) + "]");

        int result = client->getRequest()->consume(buffer);
        Logger::info("Request parsing result: " + intToStr(result) + ", state: " + intToStr(client->getRequest()->state) + " [" + intToStr(fd) + "]");
    }


    if(client->getRequest()->state == StateParsingComplete){
        Logger::info("Request parsing complete, method: " + client->getRequest()->getMethod() + ", URL: " + client->getRequest()->getUrl() + " [" + intToStr(fd) + "]");
        
        // TODO: based on the value from the config file, we need to decide if it is a valid request
        // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/16
        // eg: if the method is in the allowed methods(direttiva del config-file)
        // eg: if proxy_pass is set, i think, not sure, we need to make a send() with the request to the proxy_pass server

        // TODO: based on the URL (credo), we need to decide if we need to pass the request to CGI
        // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/15
        
        parser.validateResource(client, client->getServer());
        
        this->removeFromSet(fd, &this->_readPool);
        this->addToSet(fd, &this->_writePool);
        
        // this->debugPools("Dopo processRequest", fd);
    }
}


void ServerManager::sendErrorResponse(Response *response, SOCKET fd, Client *client) 
{
    std::string errorPage = getErrorPage(response->getStatus(), client->getServer());
    if (errorPage.empty()) {
        std::cerr << "[" << fd << "] ERROR: Error page not found" << std::endl;
        return;
    }
    response->setBody(errorPage);

    //TODO: da aggiungere il settaggio degli header (fatto a caso quello sotto però worka)
    //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/37
    response->setHeaders("Host", "localhost");
    response->setHeaders("Content-Type", "text/html");
    response->setHeaders("Content-Length", intToStr(errorPage.size()));
    response->setHeaders("Connection", "close");

    response->prepareResponse();

    std::cout << "[" << fd << "] DEBUG: Body size: " << errorPage.size() << " bytes" << std::endl;
    std::cout << "[" << fd << "] DEBUG: Total response size: " << response->getResponse().size() << " bytes" << std::endl;
    
    std::cout << "[" << fd << "] INFO: Sending ERROR response: " << std::endl;
    int bytes_sent = send(fd, response->getResponse().c_str(), response->getResponse().size(), 0);

    if (bytes_sent == -1) {
        std::cerr << "[" << fd << "] ERROR: Send failed: " << strerror(errno) << std::endl;
    }
    std::cout << "[" << fd << "] INFO: ERROR response sent successfully (" << bytes_sent << " bytes)" << std::endl;
    
    std::cout << "[" << fd << "] INFO: Closing connection as demand by ERROR" << std::endl;
    this->closeClientConnection(fd, client);
    return;
}

void ServerManager::sendResponse(SOCKET fd, Client *client)
{
    // this->debugPools("Prima di sendResponse", fd);
    
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    client->updateLastActivity();

    if (!request || !response || request->state != StateParsingComplete) {
        if (request && request->state == StateParsingError) { //gestione errori di parsing
            this->sendErrorResponse(response, fd, client);
            return ;
        }
        std::cerr << "[" << fd << "] ERROR: Cannot send response, invalid request or response state" << std::endl;
        Logger::error("ServerManager", "Cannot send response, invalid request or response state [" + intToStr(fd) + "]");
        return;
    }

    Logger::info("Preparing response for " + request->getMethod() + " " + request->getUrl() + " (Status: " + intToStr(response->getStatus()) + ") [" + intToStr(fd) + "]");

    // setta gli headers della response
    response->setHeaders("Host", "localhost");

    std::string connectionHeader = to_lower(request->getHeaders()["connection"]);
    if (connectionHeader == "keep-alive") {
        response->setHeaders("Connection", "keep-alive");
    } else {
        response->setHeaders("Connection", "close");
    }

    // se non ho il body non setto i due header
    if (!response->getBody().empty()) {
        response->setHeaders("Content-Type", getContentType(request->getUrl(), response->getStatus()));
        response->setHeaders("Content-Length", intToStr(response->getBody().size()));
    }

    response->prepareResponse();

    Logger::info("Sending response [" + intToStr(fd) + "]");

    int bytes_sent = send(fd, response->getResponse().c_str(), response->getResponse().size(), 0);

    if (bytes_sent == -1) {
        Logger::error("ServerManager", "Send failed: " + std::string(strerror(errno)) + " [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
        return;
    }

    Logger::info("Response sent successfully (" + intToStr(bytes_sent) + " bytes) [" + intToStr(fd) + "]");

    // gestione della connessione dopo l'invio della risposta
    if(connectionHeader == "close") {
        Logger::info("Closing connection as requested by client [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
    } else {
        client->reset();
        this->resetPoolForNextRequest(fd);
        
        // this->debugPools("Dopo sendResponse (keep-alive)", fd);
        
        Logger::info("Connection kept alive for next request [" + intToStr(fd) + "]");
    }
    
    return; 
}


void ServerManager::initFdSets()
{
    // aggiungo i socket dei server alla read pool
    for (std::map<SOCKET, Server*>::iterator server_it = this->_servers_map.begin(); server_it != this->_servers_map.end(); ++server_it){
        SOCKET serverSocket = server_it->first;

        this->addToSet(serverSocket, &this->_masterPool);
        this->addToSet(serverSocket, &this->_readPool);
        
        if(serverSocket > this->_maxSocket){
            this->_maxSocket = serverSocket;
        }
    }

    // aggiungo i socket dei client al master pool, ma non alla write e alla read pool
    // perchè verranno aggiunti in base al loro stato in processRequest e sendResponse
    for (std::map<SOCKET, Client*>::iterator it = this->_clients_map.begin(); it != this->_clients_map.end(); ++it){
        SOCKET clientSocket = it->first;
        Client* client = it->second;
        
        this->addToSet(clientSocket, &this->_masterPool);
        
        // se la richiesta è completa, aggiungo il socket alla write pool
        if (client->getRequest() && client->getRequest()->state == StateParsingComplete) {
            this->addToSet(clientSocket, &this->_writePool);
        } else {
            //aggiungo il socket alla read pool per ricevere nuovi dati
            this->addToSet(clientSocket, &this->_readPool);
        }
        
        this->_maxSocket = std::max(this->_maxSocket, clientSocket);
    }
}
std::map<SOCKET, Server*>	ServerManager::getServerMap(void){return this->_servers_map;}

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




