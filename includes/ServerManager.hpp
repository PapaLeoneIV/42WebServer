#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Utils.hpp"
#include "Server.hpp"
#include "Client.hpp"

class ServerManager{

    public:
    
    ServerManager();
    ~ServerManager();


    void                            mainLoop                (void);

    void                            initFdSets              (void);
    void                            registerNewConnections  (Server *server, SOCKET serverFd);
    void                            processRequest          (Client *client);
    void                            sendResponse            (SOCKET fd, Client *client);

    Client                          *getClient              (SOCKET clientFd);
    void                            removeClient            (SOCKET fd);
    void                            addServer               (Server *server);
    void                            addToSet                (SOCKET fd, fd_set *fdSet);
    void                            removeFromSet           (SOCKET fd, fd_set *fd_set);
    
    private:
    
    fd_set                          _readPool;
    fd_set                          _writePool;
    SOCKET                          _maxSocket;
    std::map<SOCKET, Client*>       _clients_map; 
    std::map<SOCKET, Server*>       _servers_map;
};


#endif