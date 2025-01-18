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
        
        std::cout << "Reading body before readBodyData" << std::endl;
        if(this->readBodyData(client) != SUCCESS){
            this->removeClient(client->getSocketFd());
            return;
        }
    }

    //split request into headers and body
    Request* request = parser.decompose(client->getHeadersData() + client->getBodyData());

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
//    removeFromSet(fd, &this->_writePool);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    fd = -1;
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

const char *ServerManager::getClientIP(Client *client)
{
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client->getAddr(), client->getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return address_info;
}

ERROR ServerManager::readHeaderData(Client *client)
{
    char headerBuff[1024]; // Use a larger buffer
    int max_attempts = 1000; // Prevent infinite loops
    int attempts = 0;

    while (true) {
        if (++attempts > max_attempts) {
            std::cerr << "Header read timeout or too many attempts" << std::endl;
            return ERR_RECV;
        }

        // Check if headers are fully received
        if (client->getHeadersData().find("\r\n\r\n") != std::string::npos) {
            break;
        }

        std::cout << "sizeof(headerBuff): " << sizeof(headerBuff) << std::endl;
        int bytes_received = recv(client->getSocketFd(), headerBuff, sizeof(headerBuff), 0);

        if (bytes_received == -1) {
            std::cerr << "headers bytes = -1" << std::endl;
            std::cerr << strerror(errno) << std::endl;
            return ERR_RECV;
        }
        if (bytes_received == 0) {
            std::cerr << "headers bytes = 0" << std::endl;
            std::cerr << strerror(errno) << std::endl;
            return ERR_RECV;
        }

        // Construct string with actual received size
        std::string received_data(headerBuff, bytes_received);
        std::string joined = client->getHeadersData() + received_data;
        client->setHeadersData(joined);
    }

    std::cout << "Headers received: " << client->getHeadersData() << std::endl;
    return SUCCESS;
}


ERROR ServerManager::readBodyData(Client *client) {


    std::cout << "Reading body" << std::endl;
    char bodyBuff[MAX_REQUEST_SIZE + 1];
    int bytes_received = recv(client->getSocketFd(), bodyBuff, sizeof(bodyBuff), 0);

    std::cout << "bytes received: " << bytes_received << std::endl;

    if(bytes_received > MAX_REQUEST_SIZE){
        std::cout << "Request too large" << std::endl;
        client->getResponse()->setStatusCode(413);
        return ERR_RECV;
    }
    if(bytes_received == -1){
        this->removeClient(client->getSocketFd());
        return ERR_RECV;
    }
    if(bytes_received == 0){
        this->removeClient(client->getSocketFd());
        return ERR_RECV;
    }
    client->setBodyData(std::string(bodyBuff));

    return SUCCESS;
}