#include "ServerManager.hpp"
#include "Booter.hpp"

ServerManager::ServerManager()
{
    this->_maxSocket = 0;
    
    FD_ZERO(&this->_readPool);
    FD_ZERO(&this->_writePool);
}

ServerManager::~ServerManager(){}


void ServerManager::mainLoop()
{
    Server *server = new Server();
    Booter booter;

    //creo i/il server
    booter.bootServer(server, "localhost", "8081");

    //aggiungo il server alla lista dei server
    this->addServer(server);

    while(1){
        
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        this->initFdSets();

        if (select(this->_maxSocket + 1, &this->_readPool, &this->_writePool, 0, &timeout) < 0){
            throw std::runtime_error(ErrToStr(ERR_SELECT));
        }

        for (SOCKET fd = 0; fd < this->_maxSocket + 1; ++fd){

            if(FD_ISSET(fd, &this->_readPool) && this->_servers_map.count(fd) > 0){
                this->registerNewConnections(this->_servers_map[fd], fd);
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

    for (std::map<SOCKET, Server*>::iterator server_it = this->_servers_map.begin(); server_it != this->_servers_map.end(); ++server_it){
        FD_SET(server_it->first, &this->_readPool);
    }

    this->_maxSocket = this->_servers_map.rbegin()->first;

    //TODO client non penso ce ne possono essere in questo momento
    for (std::map<SOCKET, Client*>::iterator clientIt = this->_clients_map.begin(); clientIt != this->_clients_map.end(); ++clientIt){
        FD_SET(clientIt->first, &this->_readPool);
        FD_SET(clientIt->first, &this->_writePool);
        this->_maxSocket = std::max(this->_maxSocket, clientIt->first);
    }
}


void ServerManager::registerNewConnections(Server *server, SOCKET serverFd)
{
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
    
    //setto il socket come non bloccante
    if(fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0){
        delete client;
        close(new_socket);
        return;
    }
    //aggiungo il socket al pool di socket da monitorare
    this->addToSet(new_socket, &this->_readPool);
    //setto il nuovo socket al client
    client->setSocketFd(new_socket);
    
    this->_clients_map[new_socket] = client;
    
    std::cout << "New connection from " << server->getClientIP(*client) << std::endl;
}

void ServerManager::processRequest(Client *client)
{
    Parser parser;

    int bytes_received = recv(client->getSocketFd(),
                              client->getRequestData() + client->getRecvBytes(),
                              MAX_REQUEST_SIZE - client->getRecvBytes(), 0);

    if(bytes_received == -1){
        std::cerr << "Error: recv failed: closing connection" << std::endl;
        this->removeClient(client->getSocketFd());
        return;
    }
    if(bytes_received == 0){
        this->removeClient(client->getSocketFd());
        return;
    }

    client->setRecvData(bytes_received + client->getRecvBytes());

    Request* request = parser.decompose(client->getRequestData());
    

    parser.parse(request, client);
    
    client->set_Request(request);

    client->getResponse()->setBody("Hello World");

    this->removeFromSet(client->getSocketFd(), &this->_readPool);
    this->addToSet(client->getSocketFd(), &this->_writePool);
}

void ServerManager::sendResponse(SOCKET fd, Client *client)
{
    if(client->getResponse()->getBody().empty()){
        client->getResponse()->setHeaders("Content-Type", getContentType(client->getRequest()->getUrl()));
        client->getResponse()->setHeaders("Content-Length", intToStr(client->getResponse()->getBody().size()));
    }
    client->getResponse()->setHeaders("Host", "localhost");
    client->getResponse()->setHeaders("Connection", "close");

    client->getResponse()->prepareResponse();

    client->getResponse()->print();

    int bytes_sent = send(fd, client->getResponse()->getResponse().c_str(), client->getResponse()->getResponse().size(), 0);

    if (bytes_sent == -1){
        std::cerr << "Error: send failed: closing connection" << std::endl;
        this->removeClient(fd);
        return;
    }

    //this->removeFromSet(fd, &this->_writePool);
    //this->addToSet(fd, &this->_readPool);   
     
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
    if(FD_ISSET(fd, &this->_readPool)){
        removeFromSet(fd, &this->_readPool);
    }
    if(FD_ISSET(fd, &this->_writePool)){
        removeFromSet(fd, &this->_writePool);
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

void ServerManager::addServer(Server *server)
{
    SOCKET serverSocket = server->getServerSocket();
    this->_servers_map[serverSocket] = server;
    this->addToSet(serverSocket, &this->_readPool);
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