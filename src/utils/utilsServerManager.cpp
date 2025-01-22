#include "ServerManager.hpp"
#include "Booter.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"


Client *ServerManager::getClient(SOCKET clientFd){
    
    //se il clientFd passato esiste tra i clienti registrati lo ritorno
    if (this->_clients_map.count(clientFd) > 0){
        return this->_clients_map[clientFd];
    }
    //altrimenti ne creo uno nuovo
    return new Client(clientFd);
}

void ServerManager::removeClient(SOCKET fd){
    if (this->_clients_map.count(fd) > 0){
        delete this->_clients_map[fd];
        this->_clients_map.erase(fd);
    }
    if(FD_ISSET(fd, &this->_masterPool)){
        removeFromSet(fd, &this->_masterPool);
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    fd = -1;
}

void ServerManager::addServer(Server *server){
    SOCKET serverSocket = server->getServerSocket();
    this->_servers_map[serverSocket] = server;
}

void ServerManager::addToSet(SOCKET fd, fd_set *fdSet){
    //aggiungo il socket al set
    FD_SET(fd, fdSet);
    //aggiorno il max socket
    this->_maxSocket = std::max(this->_maxSocket, fd);
}

void ServerManager::removeFromSet(SOCKET fd, fd_set *fd_set){
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

const char *ServerManager::getClientIP(Client *client){
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client->getAddr(), client->getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return address_info;
}

ERROR ServerManager::readHeaderData(Client *client){
    char headerBuff[1];

    while (true) {

        if (client->getHeadersData().find("\r\n\r\n") != std::string::npos) {
            break;
        }
        int bytes_received = recv(client->getSocketFd(), headerBuff, sizeof(headerBuff), 0);

        if (bytes_received == -1) {
            std::cerr << strerror(errno) << std::endl;
            return ERR_RECV;
        }
        if (bytes_received == 0) {
            std::cerr << strerror(errno) << std::endl;
            return ERR_RECV;
        }

        std::string received_data(headerBuff, bytes_received);

        std::string joined = client->getHeadersData() + received_data;
        client->setHeadersData(joined);
    }

   

    return SUCCESS;
}

ERROR ServerManager::readBodyData(Client *client) {

    std::vector<char> bodyBuff(MAX_REQUEST_SIZE + 1);
    std::vector<char> joined;
    int totReceived = 0;
    while(totReceived < client->getRequest()->getContentLength())
    {   
        int bytes_received = recv(client->getSocketFd(), bodyBuff.data(), client->getRequest()->getContentLength() - joined.size(), 0);
        if(totReceived > MAX_REQUEST_SIZE){
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
        joined.insert(joined.end(), bodyBuff.begin(), bodyBuff.begin() + bytes_received);
        totReceived += bytes_received;
    }

    joined.resize(client->getRequest()->getContentLength());
    client->setBodyData(std::string(joined.begin(), joined.end()));

    std::cout << "Body data received size: " << client->getBodyData().length() << std::endl;
   
    return SUCCESS;
}

ERROR ServerManager::readChunked(Client *client) {
    std::string line;
    std::string joined;

    std::vector<char> bodyBuff(MAX_REQUEST_SIZE + 1);
    
    int totReceived = 0;

    while (totReceived < MAX_REQUEST_SIZE) {
        
        int bytes_received = recv(client->getSocketFd(), bodyBuff.data(), MAX_REQUEST_SIZE - joined.size(), 0);
        if(bytes_received == -1){
            this->removeClient(client->getSocketFd());
            return  ERR_RECV;
        }
        if(bytes_received == 0){
            this->removeClient(client->getSocketFd());
            return ERR_RECV;
        }
        joined.insert(joined.end(), bodyBuff.begin(), bodyBuff.begin() + bytes_received);
    }
    client->getRequest()->setBody(joined);
}