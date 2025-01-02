#include "Server.hpp"

void Server::signalHandler()
{
    this->_keep_alive = false;
}

ERROR Server::handleConnections(){
    

    try{
        this->monitorSocketIO();
        
        this->registerNewConnections();

        this->serveClients();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}


void Server::serveClients() {
    ERROR error;

for (std::vector<Client*>::iterator client = this->_clients.begin(); client != this->_clients.end();) {
    if (FD_ISSET((*client)->getSocketFd(), &this->_fds)) {
        if ((error = this->_parser.checkMaxRecvBytes((*client)->getRecvBytes()))) {
            this->sendResponse(new Response(400, "Bad Request: request too long"), (*client)->getSocketFd());
            this->closeClient((*client)->getSocketFd());
            client = this->_clients.erase(client);
            continue;
        }
        int bytes_received = recv((*client)->getSocketFd(),
                                   (*client)->getRequest() + (*client)->getRecvBytes(),
                                   MAX_REQUEST_SIZE - (*client)->getRecvBytes(),
                                   0);
        if ((error = this->_parser.checkRecvBytes(bytes_received))) {
            this->sendResponse(new Response(400, "Connection closed by ..."), (*client)->getSocketFd());
            this->closeClient((*client)->getSocketFd());
            client = this->_clients.erase(client);
            continue;
        }
        (*client)->set_recv_bytes(bytes_received + (*client)->getRecvBytes());
        
        Request* request = this->_parser.parse(client);
        if ((error = this->_parser.validate(request))) {
            this->sendResponse(new Response(500, "Internal server error"), (*client)->getSocketFd());
            this->closeClient((*client)->getSocketFd());
            client = this->_clients.erase(client);
            continue;
        }
        (*client)->set_Request(request);
        this->sendResponse(new Response(200, "OK"), (*client)->getSocketFd());
        this->closeClient((*client)->getSocketFd());
        client = this->_clients.erase(client);
        continue;
    }
    ++client;
}
}


void Server::registerNewConnections()
{
    if (FD_ISSET(this->_server_socket, &this->_fds))
    {
        Client* client = this->getClient(-1);
        if (client == NULL) {
            throw std::runtime_error("Error: client allocation failed");            
        }
        
        SOCKET new_socket = accept(this->_server_socket, (sockaddr*)&client->getAddr(), &client->getAddrLen()); 
        if (new_socket < 0){
            throw std::runtime_error("Error: accept failed");
        }
        
        client->setSocketFd(new_socket);
        
        this->_clients.push_back(client);
        
        std::cout << "New connection from " << this->getClientIP(*client) << std::endl;
    }
}

void Server::monitorSocketIO()
{
    FD_ZERO(&this->_fds);  

    FD_SET(this->_server_socket, &this->_fds); 

    SOCKET max_socket = this->_server_socket;
    for (std::vector<Client*>::iterator client_it = this->_clients.begin(); client_it != this->_clients.end(); ++client_it){
        FD_SET((*client_it)->getSocketFd(), &this->_fds);
        max_socket = std::max(max_socket, (*client_it)->getSocketFd());
    }

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    if (select(max_socket + 1, &this->_fds, 0, 0, &timeout) < 0){
        throw std::runtime_error("Error: select failed");
    }
}

ERROR Server::sendResponse(Response *response, SOCKET fd) {
    std::string header;

    if (!response->getBody().empty()) {
        response->setContentLength(response->getBody().size());
    } else {
        response->setContentLength(0);
    }

    header += "HTTP/1.1 " + int_to_string(response->getStatus()) + " " + response->getStatusMessage() + "\r\n";

    header += "Content-Type: text/plain\r\n";
    header += "Content-Length: " + int_to_string(response->getContentLength()) + "\r\n";
    header += "Connection: close\r\n";


    
    std::map<std::string, std::string> headers = response->getHeaders();
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
        header += it->first + ": " + it->second + "\r\n";
    }

    header += "\r\n";

    if (!response->getBody().empty()) {
        header.append(response->getBody());
    }

    response->setResponse(const_cast<char *>(header.c_str()));

    ssize_t bytes_sent = send(fd, response->getResponse().c_str(), response->getResponse().size(), response->getFlags());

    if (bytes_sent == -1) {
        throw std::runtime_error("Error: send failed");
    }

    return 0;
}

void Server::closeClient(SOCKET fd) {
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

Client *Server::getClient(SOCKET fd)
{
    std::vector<Client*>::iterator client_it;
    
    for(client_it = this->_clients.begin(); client_it != this->_clients.end(); client_it++){
        if((*client_it)->getSocketFd() == fd) 
            return (*client_it);
    }

    Client *new_client = new Client();
    
    new_client->setSocketFd(fd);
    new_client->setAddrLen(sizeof(new_client->getAddr()));
    
    return new_client;
}

const char *Server::getClientIP(Client client)
{
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client.getAddr(), client.getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return address_info;
}


//GETTERS
SOCKET                       Server::getServerSocket() { return this->_server_socket;}
fd_set                       Server::getFdsSet() { return this->_fds;}
addrinfo                     &Server::getHints() { return this->_hints;}
addrinfo                     *Server::getBindAddrss() { return this->_bind_address;}

//SETTERS
void                         Server::setServerSocket(SOCKET server_socket){this->_server_socket = server_socket;};
void                         Server::setFds(fd_set fds){this->_fds = fds;};
void                         Server::setHints(addrinfo hints){this->_hints = hints;};
void                         Server::setBindAddress(addrinfo *bind_address){this->_bind_address = bind_address;};

Server::Server(): _clients(),_keep_alive(true), _hints(), _server_socket(-1), _bind_address(0) {

    this->_hints.ai_family = AF_INET;      
    this->_hints.ai_socktype = SOCK_STREAM;
    this->_hints.ai_flags = AI_PASSIVE;

}

Server::~Server(){}