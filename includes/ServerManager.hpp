#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP


#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <algorithm>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <set>

#include "Server.hpp"
#include "Client.hpp"

typedef int SOCKET;
typedef int ERROR;

class ServerManager{

	public:

	ServerManager();
	~ServerManager();


	void                            mainLoop                (void);

	void                            initFdSets              (void);
	void                            registerNewConnections  (SOCKET serverFd, Server *server);
	void                            processRequest          (Client *client);
	void                            sendResponse            (SOCKET fd, Client *client);

	ERROR							readHeaderData (Client *client);
	ERROR							readBodyData (Client *client);
	ERROR    						readChunked (Client *client);
	Client                         *getClient               (SOCKET clientFd);
	const char *                    getClientIP             (Client *client);
	void                            removeClient            (SOCKET fd);
	void                            addServer               (Server *server);
	void                            addToSet                (SOCKET fd, fd_set *fdSet);
	void                            removeFromSet			(SOCKET fd, fd_set *fd_set);
	

	
	private:

	fd_set                          _masterPool;
	fd_set                          _readPool;
	fd_set                          _writePool;

	SOCKET                          _maxSocket;

	std::map<SOCKET, Client*>       _clients_map;
	std::map<SOCKET, Server*>       _servers_map;
};


#endif