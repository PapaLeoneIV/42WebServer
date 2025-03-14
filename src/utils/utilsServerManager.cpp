#include "../../includes/ServerManager.hpp"
#include "../../includes/Booter.hpp"
#include "../../includes/Parser.hpp"
#include "../../includes/Request.hpp"
#include "../../includes/Response.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Utils.hpp"
#include "../../includes/Logger.hpp"

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

void ServerManager::resetPoolForNextRequest(SOCKET fd){
    // this->debugPools("Prima di resetPoolForNextRequest", fd);
    
    this->removeFromSet(fd, &this->_writePool);
    this->addToSet(fd, &this->_readPool);
    
    // this->debugPools("Dopo resetPoolForNextRequest", fd);
}

void ServerManager::debugPools(const std::string& label, SOCKET fd) {
    bool inReadPool = FD_ISSET(fd, &this->_readPool);
    bool inWritePool = FD_ISSET(fd, &this->_writePool);
    bool inMasterPool = FD_ISSET(fd, &this->_masterPool);
    
    std::cout << "[" << fd << "] DEBUG: " << label 
              << " - readPool: " << (inReadPool ? "SI" : "NO")
              << ", writePool: " << (inWritePool ? "SI" : "NO")
              << ", masterPool: " << (inMasterPool ? "SI" : "NO") << std::endl;
}

const std::string ServerManager::getClientIP(Client *client){
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client->getAddr(), client->getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return std::string(address_info);
}

void ServerManager::closeClientConnection(SOCKET fd, Client* client) 
{
    if (client == NULL) {
        // Se il client non esiste, chiudo solo il socket
        if (fd != -1) {
            removeFromSet(fd, &this->_masterPool);
            removeFromSet(fd, &this->_readPool);
            removeFromSet(fd, &this->_writePool);
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        return;
    }

    // Rimuovo il socket da tutti i set
    removeFromSet(fd, &this->_masterPool);
    removeFromSet(fd, &this->_readPool);
    removeFromSet(fd, &this->_writePool);
    
    // Chiudo il socket solo se è ancora valido
    int socket_status;
    socklen_t len = sizeof(socket_status);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &socket_status, &len) == 0) {
        // Il socket è ancora valido, possiamo chiuderlo
        if (shutdown(fd, SHUT_RDWR) < 0 && errno != ENOTCONN) {
            Logger::error("ServerManager", "Error shutting down socket: " + std::string(strerror(errno)));
        }
        if (close(fd) < 0) {
            Logger::error("ServerManager", "Error closing socket: " + std::string(strerror(errno)));
        }
    } else {
        // Il socket non è più valido, probabilmente già chiuso
        Logger::info("Socket already closed [" + intToStr(fd) + "]");
    }
    
    // Rimuovo il client dalla mappa e lo dealloco
    this->_clients_map.erase(fd);
    delete client;
    
}


void ServerManager::handleClientTimeout(time_t currentTime) {
    std::vector<SOCKET> clientsToRemove;
    
    // Identifico i client che hanno superato il timeout
    for (std::map<SOCKET, Client*>::iterator it = this->_clients_map.begin(); it != this->_clients_map.end(); ++it) {
        Client* client = it->second;
        // Se il client non ha avuto attività per più di TIMEOUT_SEC sec, lo rimuovo
        if (currentTime - client->getLastActivity() > TIMEOUT_SEC) {
            clientsToRemove.push_back(it->first);
        }
    }
    
    // Rimuovo i client che hanno superato il timeout
    for (std::vector<SOCKET>::iterator it = clientsToRemove.begin(); it != clientsToRemove.end(); ++it) {
        SOCKET fd = *it;
        Client* client = this->_clients_map[fd];
        Logger::info("Connection timeout, closing connection [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
    }
}

// ERROR ServerManager::readHeaderData(Client *client){
//     char headerBuff[1];


//     //TODO understand if i can do only one read operation at the time
//     while (true) {

//         if (client->getHeadersData().find("\r\n\r\n") != std::string::npos) {
//             break;
//         }
//         int bytes_received = recv(client->getSocketFd(), headerBuff, sizeof(headerBuff), 0);

//         if (bytes_received == -1) {
//             std::cerr << strerror(errno) << std::endl;
//             return ERR_RECV;
//         }
//         if (bytes_received == 0) {
//             std::cerr << strerror(errno) << std::endl;
//             return ERR_RECV;
//         }

//         std::string received_data(headerBuff, bytes_received);

//         std::string joined = client->getHeadersData() + received_data;
//         client->setHeadersData(joined);
//     }


//     return SUCCESS;
// }




// ERROR ServerManager::handleTransferLength(Client *client) {

//     std::vector<char> bodyBuff(MAX_REQUEST_SIZE + 1);
//     std::vector<char> joined;
//     int totReceived = 0;
//     while(totReceived < client->getRequest()->getContentLength())
//     {   
//         int bytes_received = recv(client->getSocketFd(), bodyBuff.data(), client->getRequest()->getContentLength() - joined.size(), 0);
//         if(totReceived > MAX_REQUEST_SIZE){
//             std::cout << "Request too large" << std::endl;
//             client->getResponse()->setStatusCode(413);
//             return ERR_RECV;
//         }
//         if(bytes_received == -1){
//             this->removeClient(client->getSocketFd());
//             return ERR_RECV;
//         }
//         if(bytes_received == 0){
//             this->removeClient(client->getSocketFd());
//             return ERR_RECV;
//         }
//         joined.insert(joined.end(), bodyBuff.begin(), bodyBuff.begin() + bytes_received);
//         totReceived += bytes_received;
//     }

//     joined.resize(client->getRequest()->getContentLength());
//     client->setBodyData(std::string(joined.begin(), joined.end()));

//     std::cout << "Body data received size: " << client->getBodyData().length() << std::endl;
   
//     return SUCCESS;
// }



//TODO not properly implemented at the moment
//Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/21
// ERROR ServerManager::handkeChunkedTransfer(Client *client) {
//     std::string line;
//     std::string joined;

//     std::vector<char> bodyBuff(MAX_REQUEST_SIZE + 1);
    
//     int totReceived = 0;

//     while (totReceived < MAX_REQUEST_SIZE) {
//         int bytes_received = recv(client->getSocketFd(), bodyBuff.data(), MAX_REQUEST_SIZE - joined.size(), 0);
//         if(bytes_received == -1){
//             this->removeClient(client->getSocketFd());
//             return  ERR_RECV;
//         }
//         if(bytes_received == 0){
//             this->removeClient(client->getSocketFd());
//             return ERR_RECV;
//         }
//         joined.insert(joined.end(), bodyBuff.begin(), bodyBuff.begin() + bytes_received);
//     }
//     client->getRequest()->setBody(joined);
//     return SUCCESS;
// }
