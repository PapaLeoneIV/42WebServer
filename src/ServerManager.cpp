#include "ServerManager.hpp"
#include "Booter.hpp"

ServerManager::ServerManager()
{
    this->_maxSocket = 0;
    
    FD_ZERO(&this->_readPool);
    FD_ZERO(&this->_writePool);
    FD_ZERO(&this->_masterPool);
}

ServerManager::~ServerManager(){}


void ServerManager::mainLoop()
{
    Server *server = new Server();
    Booter booter;

    //PARSE CONFIG FILE HERE
    
    //std::vector<Server*> = parser.parseConfigFile(argv[2]);

    //creo i/il server
    booter.bootServer(server, "10.11.2.2", "8080");

    //aggiungo il server alla lista dei server
    
    this->addServer(server);
    
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

        for (SOCKET fd = 0; fd <= this->_maxSocket; ++fd){

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

    std::cout << "New connection from " << this->getClientIP(*client) << std::endl;
    
    //setto il socket come non bloccante
    if(fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0){
        delete client;
        close(new_socket);
        return;
    }
    //aggiungo il socket al pool di socket da monitorare
    this->addToSet(new_socket, &this->_masterPool);
    //setto il nuovo socket al client
    client->setSocketFd(new_socket);
    //aggiungo un riferimento al server all interno del client
    client->setServer(server);
    
    this->_clients_map[new_socket] = client;
}


void ServerManager::processRequest(Client *client)
{
    Parser parser;

    //read from client socket
    int bytes_received = recv(client->getSocketFd(),
                              client->getRequestData() + client->getRecvBytes(),
                              MAX_REQUEST_SIZE - client->getRecvBytes(), 0);

    if(bytes_received == -1){
        this->removeClient(client->getSocketFd());
        return;
    }

    if(bytes_received == 0){
        this->removeClient(client->getSocketFd());
        return;
    }
    //request size is too large for what the server is willing to accept
    //TODO get the MAX_REQUEST_SIZE from the server configuration
    if(bytes_received > MAX_REQUEST_SIZE - client->getRecvBytes()){
        client->getResponse()->setStatusCode(413);
        return;
    }

    client->setRecvData(bytes_received + client->getRecvBytes());

    //split request into headers and body
    Request* request = parser.decompose(client->getRequestData());
        
    client->set_Request(request);
    
    //once the request is parsed, we can try to validate and in case of error we start to fill the client response
    if(parser.parse(request, client) != SUCCESS)
        return;

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
    } else{
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


Client *ServerManager::getClient(SOCKET clientFd)
{
    
    //se il clientFd passato esiste tra i clienti registrati lo ritorno
    if (this->_clients_map.count(clientFd) > 0){
        return this->_clients_map[clientFd];
    }
    //altrimenti ne creo uno nuovo
    return new Client(clientFd);
}

void ServerManager::removeClient(SOCKET fd)
{
    if (this->_clients_map.count(fd) > 0){
        delete this->_clients_map[fd];
        this->_clients_map.erase(fd);
    }
    if(FD_ISSET(fd, &this->_masterPool)){
        removeFromSet(fd, &this->_masterPool);
    }
    /* if(FD_ISSET(fd, &this->_readPool)){
        removeFromSet(fd, &this->_readPool);
    }
    if(FD_ISSET(fd, &this->_writePool)){
    } */
    removeFromSet(fd, &this->_writePool);
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

void ServerManager::addServer(Server *server)
{
    SOCKET serverSocket = server->getServerSocket();
    this->_servers_map[serverSocket] = server;
}

void ServerManager::addToSet(SOCKET fd, fd_set *fdSet)
{
    //aggiungo il socket al set
    FD_SET(fd, fdSet);
    //aggiorno il max socket
    this->_maxSocket = std::max(this->_maxSocket, fd);
}

void ServerManager::removeFromSet(SOCKET fd, fd_set *fd_set)
{
    //rimuovo il socket dal set
    FD_CLR(fd, fd_set);
    if (fd == this->_maxSocket){
        for (SOCKET i = this->_maxSocket - 1; i >= 0; i--){
            //dopo aver rimosso il socket dal set aggiorno il max socket
            if (FD_ISSET(i, fd_set)){
                this->_maxSocket = i;
                break;
            }
        }
    }
}

const char *ServerManager::getClientIP(Client client)
{
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client.getAddr(), client.getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return address_info;
}
