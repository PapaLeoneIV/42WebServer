#include "ServerManager.hpp"
#include "Booter.hpp"

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
    //TODO client non penso ce ne possono essere in questo momento
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

void ServerManager::processRequest(Client *client)
{
    Parser parser;

    //I decided to split the reading of the headers and the body in two different functions
    //because the headers part does not have a limit but the body part yes
    //read headers
    if(this->readHeaderData(client) != SUCCESS){
        this->removeClient(client->getSocketFd());
        return;
    }

    

    //read_body if we find header indicating the presence of a body
    if(client->getHeadersData().find("Content-Length") != std::string::npos ||
        client->getHeadersData().find("Transfer-Encoding") != std::string::npos ||
        client->getHeadersData().find("Content-Type") != std::string::npos){

            if(client->getHeadersData().find("Content-Length") != std::string::npos){
                int contLength = strToInt(client->getHeadersData().substr(client->getHeadersData().find("Content-Length") + 16));
                if(this->readBodyData(client, contLength) != SUCCESS){
                    this->removeClient(client->getSocketFd());
                    return;
                }
            }
        }
    std::cout << "Headers data received: client->getHeadersData() " << client->getHeadersData() << std::endl;
    std::cout << "Body data received: client->getBodyData()" << client->getBodyData() << std::endl;

    //split request into headers and body
    Request* request = parser.decompose(client->getHeadersData(), client->getBodyData(), client);

    client->set_Request(request);

    //once the request is parsed, we can try to validate and in case of error we start to fill the client response
    //parser.parse(request, client);
    //TODO understand why if i dont responde immediatly the client closes the connection
    if(parser.parse(request, client) != SUCCESS)
    {
        client->getResponse()->setBody(client->getResponse()->getErrorPage(client->getResponse()->getStatus()));
         this->sendResponse(client->getSocketFd(), client); 
        return;
    }

    parser.validateResource(client, client->getServer());
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

    //response->print();

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

