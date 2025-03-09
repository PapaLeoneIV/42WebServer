#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"

//GETTERS
SOCKET      Server::getServerSocket()	{return this->_server_socket;}
fd_set      Server::getFdsSet()	{return this->_fds;}
addrinfo    &Server::getHints()	{return this->_hints;}
addrinfo    *Server::getBindAddrss()	{return this->_bind_address;}
std::string &Server::getHost()	{return this->_host;};
std::string &Server::getServerName()	{return this->_server_name;};
std::string &Server::getPort()	{return this->_port;};
std::string &Server::getRoot()	{return this->_root;};
std::string &Server::getIndex()	{return this->_index;};
size_t      &Server::getMaxRequestSize(){return this->_max_request_size;};
std::string &Server::getCwd()	{return this->_cwd;};

std::map<std::string, std::vector<std::string> > Server::getServerDir(void) {return this->serverDir;}
std::map<std::string, std::map<std::string, std::vector<std::string> > > Server::getLocationDir(void) {return this->locationDir;} 



//SETTERS
void    Server::setServerSocket(SOCKET server_socket)   {this->_server_socket = server_socket;};
void    Server::setFds(fd_set fds)	{this->_fds = fds;};
void    Server::setHints(addrinfo hints)	{this->_hints = hints;};
void    Server::setBindAddress(addrinfo *bind_address)  {this->_bind_address = bind_address;};
void    Server::setHost(std::string host)   {this->_host = host;};
void    Server::setServerName(std::string server_name)  {this->_server_name = server_name;};
void    Server::setPort(std::string port)   {this->_port = port;};
void    Server::setRoot(std::string root)   {this->_root = root;};
void    Server::setIndex(std::string index) {this->_index = index;};
void    Server::setMaxRequestSize(size_t max_request_size) {this->_max_request_size = max_request_size;};
void    Server::setCwd(std::string cwd) {this->_cwd = cwd;};

void	Server::setServerDir(std::string key, std::vector<std::string> value) {
	this->serverDir[key] = value;
}

void	Server::setLocationDir(std::string locationPath, std::string key,  std::vector<std::string> value) {
	this->locationDir[locationPath][key] = value;
}


Server::Server(): _hints(), _server_socket(-1), _bind_address(NULL) {

	this->_cwd = getcwd(NULL, 0);
	this->_max_request_size = MAX_REQUEST_SIZE;
	this->_hints.ai_family = AF_INET;      
	this->_hints.ai_socktype = SOCK_STREAM;
	this->_hints.ai_flags = AI_PASSIVE;

}

Server::~Server(){}