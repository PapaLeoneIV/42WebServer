#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <string>
#include <map>
#include "Utils.hpp"
#include "Server.hpp"
#include "Client.hpp"

typedef int SOCKET;
typedef int ERROR;

class ServerManager{

	public:

	ServerManager();
	~ServerManager();


	void	eventLoop	(void);
	void	initFdSets	(void);
	void	registerNewConnections	(SOCKET serverFd, Server *server);
	void	processRequest	(Client *client);
	void	sendResponse	(SOCKET fd, Client *client);
	void	sendErrorResponse(Response *response, SOCKET fd);

	ERROR	readHeaderData 	(Client *client);
	ERROR	readBodyData	(Client *client);
	ERROR	handleTransferLength (Client *client);
	ERROR	handkeChunkedTransfer (Client *client);
	Client	*getClient	(SOCKET clientFd);
	const char	*getClientIP	(Client *client);
	void	removeClient	(SOCKET fd);
	void	addServer	(Server *server);
	void	addToSet	(SOCKET fd, fd_set *fdSet);
	void	removeFromSet	(SOCKET fd, fd_set *fd_set);

	std::map<SOCKET, Server*>	getServerMap(void);
	

	
	private:

	struct timeval timeout;
	
	fd_set                          _masterPool;
	fd_set                          _readPool;
	fd_set                          _writePool;

	SOCKET                          _maxSocket;

	std::map<SOCKET, Client*>       _clients_map;
	std::map<SOCKET, Server*>       _servers_map;
};


#endif