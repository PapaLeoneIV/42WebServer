#include "../includes/Booter.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/ServerManager.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Cgi.hpp"

/**
 * Event loop, il programma tramite la funzione bloccante select(), monitora
 * i sockets nelle varie pool (read/write). Se uno degl fd switcha stato(I/O) select()
 * esce e la Request viene gestita nel loop interno.
*/
void ServerManager::eventLoop()
{
    while(420){
        int fds_changed = 0;
        
        FD_ZERO(&this->_readPool);
        FD_ZERO(&this->_writePool);
        
        this->initFdSets();

        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        time_t currentTime = time(NULL);
        this->handleClientTimeout(currentTime);

        if ((fds_changed = select(this->_maxSocket + 1, &this->_readPool, &this->_writePool, 0, &timeout)) < 0){
            Logger::error("ServerManager", "Error in select(): " + std::string(ErrToStr(ERR_SELECT)));
            continue;
        }
        if(fds_changed == 0) continue;

        for (SOCKET fd = 0; fd <= this->_maxSocket + 1; ++fd){
            bool clientRegistred = false;
            for(size_t i = 0; i < this->_servers_map.size(); i++){
                if(FD_ISSET(fd, &this->_readPool) && this->_servers_map[i]->getServerSocket() == fd && !clientRegistred){
                    this->registerNewConnections(fd, this->_servers_map[i]);
                    clientRegistred = true;
                }
            }
            if(FD_ISSET(fd, &this->_readPool) && this->_clients_map.count(fd) > 0){
                this->processRequest(this->_clients_map[fd]);
            }
           
            if(FD_ISSET(fd, &this->_writePool) && this->_clients_map.count(fd) > 0){
                Cgi *cgi_obj = this->_clients_map[fd]->getCgiObj();
                //cgi_state = 0 la richiesta non è indirizzata a un cgi;
                //cgi_state = 1 la richiesta era indirizzata a un cgi e ho aperto il processo
                //cgi_state = 2 la richiesta era indirizzata a un cgi e ho chiuso il processo
                int cgi_state = cgi_obj->getCGIState();
                if (cgi_state == 1 && FD_ISSET(cgi_obj->getPipeOut()[1], &this->_writePool))
                    sendCgiBody(fd, _clients_map[fd], cgi_obj);
                else if (cgi_state == 1 && FD_ISSET(cgi_obj->getPipeOut()[0], &this->_readPool))
                    readCgiResponse(fd, _clients_map[fd], cgi_obj);
                else if ((cgi_state == 0 || cgi_state == 2)  && FD_ISSET(fd, &this->_writePool))
                    this->sendResponse(fd, this->_clients_map[fd]);
            }
        }            
    }
}


void ServerManager::sendCgiBody(SOCKET fd, Client *client, Cgi *cgi_obj){
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    if (!request || !response) {
        Logger::error("ServerManager", "Invalid request or response state [" + intToStr(fd) + "]");
        return;
    }
    
    int bytes_sent;

    if (request->getBody().length() == 0)
        bytes_sent = 0;
    else if (request->getBody().length() >= MAX_REQUEST_SIZE)
        bytes_sent = write(cgi_obj->getPipeIn()[1], request->getBody().c_str(), MAX_REQUEST_SIZE);
    else
        bytes_sent = write(cgi_obj->getPipeIn()[1], request->getBody().c_str(), request->getBody().length());

    if (bytes_sent < 0)
    {
        Logger::error(__FILE__,  "sendCgiBody() Error while sending CGI data to CGI process");
        removeFromSet(cgi_obj->getPipeIn()[1], &this->_writePool);
        close(cgi_obj->getPipeIn()[1]);
        close(cgi_obj->getPipeOut()[1]);
        response->setStatusCode(500);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
    }
    else if (bytes_sent == 0 || (size_t) bytes_sent == request->getBody().length())
    {
        removeFromSet(cgi_obj->getPipeIn()[1], &this->_writePool);
        close(cgi_obj->getPipeIn()[1]);
        close(cgi_obj->getPipeOut()[1]);
    }
    else
    {
        client->updateLastActivity();
    }
}

void ServerManager::readCgiBody(SOCKET fd, Client *client, Cgi *cgi_obj){
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    if (!request || !response) {
        Logger::error("ServerManager", "Invalid request or response state [" + intToStr(fd) + "]");
        return;
    }
    char buffer[BUFFER_SIZE + 1];
    int bytes_read = read(cgi_obj->getPipeOut()[0], buffer, BUFFER_SIZE);
     
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

void ServerManager::processRequest(Client *client)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    
    
    char buffer[BUFFER_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));
    
    SOCKET fd = client->getSocketFd();
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
        request->consume(buffer);
    }

    //TODO: move MACRO to a variable and update it with the server config
    if(request->getBodyCounter() > MAX_REQUEST_SIZE){
        Logger::error("ServerManager", "Request body too large [" + intToStr(fd) + "]");
        response->setStatusCode(413);
        request->setState(StateParsingError);
        return;
    }

    if(request->getState() == StateParsingComplete || request->getState() == StateParsingError){
        Logger::info("Request was consumed assigning server [" + intToStr(fd) + "]");    
        this->assignServer(client);
        if(request->getState() == StateParsingComplete) {
            Logger::info("Request parsing complete, validating resource [" + intToStr(fd) + "]");
            Parser parser;
            parser.validateResource(client, client->getServer());
            if(response->getStatus() != 200){
                this->removeFromSet(fd, &this->_readPool);
                this->addToSet(fd, &this->_writePool);
                return;
            }
        }
        if(client->getCgiObj()->getCGIState() == 1 && request->getState() == StateParsingComplete){
            Logger::info("Request is a CGI request [" + intToStr(fd) + "]");
            this->addToSet(client->getCgiObj()->getPipeOut()[0], &this->_readPool);
            this->addToSet(client->getCgiObj()->getPipeOut()[1], &this->_writePool);
            return;
        }
        if(request->getState() == StateParsingError){
            Logger::error("ServerManager", "Error parsing request [" + intToStr(fd) + "]");
            response->setStatusCode(request->getError());
            response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        }
        this->removeFromSet(fd, &this->_readPool);
        this->addToSet(fd, &this->_writePool);
    }

    client->updateLastActivity();
    return;
}

void ServerManager::assignServer(Client *client){
    
    Request *request = client->getRequest();
    
    if(request->getHeaders().find("host") != request->getHeaders().end()){
        
        std::string reqPort = request->getHeaders()["host"].find_last_of(":") != std::string::npos 
        ? 
        request->getHeaders()["host"].substr(request->getHeaders()["host"].find_last_of(":") + 1, request->getHeaders()["host"].size()) 
        : 
        "80";

        std::string reqHost = request->getHeaders()["host"].find_last_of(":") != std::string::npos 
        ? 
        request->getHeaders()["host"].substr(0, request->getHeaders()["host"].find_last_of(":"))
        :
        request->getHeaders()["host"];

        std::string reqHostPort = reqHost + ":" + reqPort;
        for(size_t i = 0; i < this->_servers_map.size(); i++){
            std::map<std::string, std::vector<std::string> > serverDir = this->_servers_map[i]->getServerDir();
            if(serverDir.find("server_name") != serverDir.end()
                && serverDir["server_name"][0] == request->getHeaders()["host"]
                && this->_servers_map[i]->getHostPortKey() ==  reqHostPort ){
                client->setServer(this->_servers_map[i]);
                break;
            }
        }
    }
}

void ServerManager::sendResponse(SOCKET fd, Client *client)
{
    // this->debugPools("Prima di sendResponse", fd);
    
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    client->updateLastActivity();

    if (!request || !response) {
        Logger::error("ServerManager", "Invalid request or response state [" + intToStr(fd) + "]");
        return;
    }


    // setta gli headers della response
    response->setHeaders("Host", "localhost");

    std::string connectionHeader = to_lower(request->getHeaders()["connection"]);
    if (connectionHeader == "close") {
        response->setHeaders("Connection", "close");
    } else {
        response->setHeaders("Connection", "keep-alive");
    }

    // se non ho il body non setto i due header
    if (!response->getBody().empty()) {
        response->setHeaders("Content-Type", getContentType(request->getUrl(), response->getStatus()));
        response->setHeaders("Content-Length", intToStr(response->getBody().size()));
    }else {
        response->setHeaders("Content-Type", "text/html");
        response->setHeaders("Content-Length", "0");
    }

    response->prepareResponse();

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
        this->closeClientConnection(fd, client);
        
        // this->resetPoolForNextRequest(fd);
        
        // this->debugPools("Dopo sendResponse (keep-alive)", fd);
        
        Logger::info("Connection kept alive for next request [" + intToStr(fd) + "]");
    }
    
    return; 
}


void ServerManager::initFdSets()
{
    // aggiungo i socket dei server alla read pool
    for (size_t i = 0; i < this->_servers_map.size(); i++){
        SOCKET serverSocket = this->_servers_map[i]->getServerSocket();

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
        if (client->getRequest() && client->getRequest()->getState() == StateParsingComplete) {
            this->addToSet(clientSocket, &this->_writePool);
        } else {
            //aggiungo il socket alla read pool per ricevere nuovi dati
            this->addToSet(clientSocket, &this->_readPool);
        }
        
        this->_maxSocket = std::max(this->_maxSocket, clientSocket);
    }
}
std::vector<Server*>	ServerManager::getServerMap(void){return this->_servers_map;}

ServerManager::ServerManager()
{
    this->_maxSocket = 0;
    FD_ZERO(&this->_readPool);
    FD_ZERO(&this->_writePool);
    FD_ZERO(&this->_masterPool);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

}

ServerManager::~ServerManager(){
    for (std::map<SOCKET, Client*>::iterator it = this->_clients_map.begin(); it != this->_clients_map.end(); ++it){
        delete it->second;
    }
    for (size_t i  = 0; i < this->_servers_map.size(); ++i){
        delete this->_servers_map[i];
    }
}




