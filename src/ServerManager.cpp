
#include "Booter.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ServerManager.hpp"
#include "Utils.hpp"
#include <algorithm>

void ServerManager::mainLoop()
{
    Server *server = new Server();
    Booter booter;

    //PARSE CONFIG FILE HERE
    
    //std::vector<Server*> = parser.parseConfigFile(argv[2]);

    //creo i/il server
    booter.bootServer(server, "localhost", "8080");

    //aggiungo il server alla lista dei server
    
    this->addServer(server);

    this->initFdSets();
   
    while(420){
        
    
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        FD_ZERO(&this->_readPool);
        FD_ZERO(&this->_writePool);

        this->_readPool = this->_masterPool;
        this->_writePool = this->_masterPool;
        
       if (select(this->_maxSocket + 1, &this->_readPool, &this->_writePool, 0, &timeout) < 0){
            throw std::runtime_error(ErrToStr(ERR_SELECT));
        }
        int max = this->_maxSocket;
        for (SOCKET fd = 0; fd <= max; ++fd){

            if(FD_ISSET(fd, &this->_readPool) && this->_servers_map.count(fd) > 0){
                this->registerNewConnections(fd, this->_servers_map[fd]);
            }
            else if(FD_ISSET(fd, &this->_readPool) && this->_clients_map.count(fd) > 0){
                this->processRequest(this->_clients_map[fd]);
            }
            else if(FD_ISSET(fd, &this->_writePool) && this->_clients_map.count(fd) > 0){
                this->sendResponse(fd, this->_clients_map[fd]);
            }
        }
    }
}

void ServerManager::initFdSets()
{
   FD_ZERO(&this->_readPool);
    FD_ZERO(&this->_writePool);
    FD_ZERO(&this->_masterPool);

    for (std::map<SOCKET, Server*>::iterator server_it = this->_servers_map.begin(); server_it != this->_servers_map.end(); ++server_it){
        FD_SET(server_it->first, &this->_masterPool);
        if(server_it->first > this->_maxSocket){
            this->_maxSocket = server_it->first;
        }
    }
    // TODO: client non penso ce ne possono essere in questo momento
    for (std::map<SOCKET, Client*>::iterator clientIt = this->_clients_map.begin(); clientIt != this->_clients_map.end(); ++clientIt){
        FD_SET(clientIt->first, &this->_masterPool);
        this->_maxSocket = std::max(this->_maxSocket, clientIt->first);
    }

}


void ServerManager::registerNewConnections(SOCKET serverFd, Server *server)
{
    // if(FD_ISSET(serverFd, &this->_readPool)){
    //creo un nuovo client
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

    std::cout << "New connection from " << this->getClientIP(client) << std::endl;
    
    //setto il socket come non bloccante
    if(fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0){
        std::cout << "Error: fcntl failed" << std::endl;
        delete client;
        close(new_socket);
        return;
    }

    //setto il nuovo socket al client
    client->setSocketFd(new_socket);
    //aggiungo il socket al pool di socket da monitorare
    this->addToSet(new_socket, &this->_masterPool);
    //aggiungo un riferimento al server all interno del client
    client->setServer(server);
    
    this->_clients_map[new_socket] = client;

}


ERROR ServerManager::readBodyData(Client *client){
    Response *response = client->getResponse();
    Request *request = client->getRequest();  
    std::string headersStream = client->getHeadersData();
    size_t valueIdx; 
    

    //std::cout << "Request headers" << headersStream << std::endl;
    //read body using Content Length
    if(headersStream.find("POST") != std::string::npos){
        if((valueIdx = headersStream.find("Content-Length")) != std::string::npos){
            valueIdx += 16;
            std::string str = headersStream.substr(valueIdx, headersStream.find("\r\n", valueIdx) - valueIdx);
            std::cout << "Content-Length: " << str << std::endl;
            int contLenValue = strToInt(str);
    
            request->setContentLength(contLenValue);
            
            if(this->handleTransferLength(client) != SUCCESS){
                this->removeClient(client->getSocketFd());
                return 1;
            }
        } else if ((valueIdx = (headersStream.find("Transfer-Encoding") + 19)) != std::string::npos){
    
            std::string encoding = headersStream.substr(valueIdx, headersStream.find("\r\n", valueIdx) - valueIdx);
            //TODO transfer encoding other than chuncked are not supported yet
            if(encoding != "chunked"){
                std::cerr << "Error: Other Transfer encoding not supported yet!" << std::endl;
                this->removeClient(client->getSocketFd());
                return 1 ;
            }
            if(this->handkeChunkedTransfer(client) != SUCCESS){
                this->removeClient(client->getSocketFd());
                return 1;
            }
        } else {
            std::cerr << "Error: No content length or transfer encoding found!" << std::endl;
            response->setStatusCode(411);
            return 1;
        }
    }
    
    return 0;
}

#define BUFFER_SIZE 1024

void ServerManager::processRequest(Client *client)
{
    Parser parser;
    Response *response = client->getResponse();
    char buffer[BUFFER_SIZE];
    int bytesRecv = recv(client->getSocketFd(), buffer, sizeof(buffer), 0);
    if(bytesRecv == -1){
        std::cerr << "Error: recv failed: closing connection" << std::endl;
        this->removeClient(client->getSocketFd());
        return;
    }
    if(bytesRecv == 0){
        std::cerr << "Error: recv failed: closing connection" << std::endl;
        this->removeClient(client->getSocketFd());
        return;
    }
    if(bytesRecv > 0){
        client->getRequest()->feed(buffer);
    }

    if(client->getRequest()->state == PARSING_COMPLETE){
        client->getRequest()->setBody(client->getRequest()->tmpBody);

    }

    
    //I decided to split the reading of the headers and the body in two different functions
    //because the headers part does not have a limit but the body part yes
    // if(this->readHeaderData(client) != SUCCESS){
    //     this->removeClient(client->getSocketFd());
    //     return;
    // }

    // if(this->readBodyData(client) != SUCCESS){
    //     this->removeClient(client->getSocketFd());
    //     return;
    // }
   
    // Request* request = parser.extract(client->getHeadersData(), client->getBodyData(), client);


    
    // if(request == NULL){ //errore during request processing
    //     response->setBody(response->getErrorPage(response->getStatus()));
    //     this->sendResponse(client->getSocketFd(), client); 
    //     return;
    // }

    // client->set_Request(request);

    // if(request->getMethod() == "GET" || request->getMethod() == "DELETE"){
        
    //     parser.validateResource(client, client->getServer());
    // } else if (request->getMethod() == "POST"){
    //     // TODO: implement POST
    // } else {
    //     response->setStatusCode(405);
    // }
    
}

void ServerManager::sendResponse(SOCKET fd, Client *client)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response){
        return;
    }

    if(!response->getBody().empty()){
        response->setHeaders("Content-Type", getContentType(request->getUrl(), response->getStatus()));
        response->setHeaders("Content-Length", intToStr(response->getBody().size()));
    } else {
        response->setHeaders("Content-Length", "0");
    }

    response->setHeaders("Host", "localhost");

    response->setHeaders("Connection", "close");

    response->prepareResponse();

    response->print();

    int bytes_sent = send(fd, response->getResponse().c_str(), response->getResponse().size(), 0);

    if (bytes_sent == -1){
        std::cerr << "Error: send failed: closing connection" << std::endl;
        this->removeClient(fd);
        return;
    }

    this->removeClient(fd);
    
    return; 
}

ServerManager::ServerManager()
{
    this->_maxSocket = 0;
    
    FD_ZERO(&this->_readPool);
    FD_ZERO(&this->_writePool);
    FD_ZERO(&this->_masterPool);
}

ServerManager::~ServerManager(){}

