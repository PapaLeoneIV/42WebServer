#include "Client.hpp"
#include "Utils.hpp"

Client::Client(Client & other) {
    
    this->_address_length = other._address_length;
    this->_socket = other._socket;
    this->_received = other._received;
    this->_address = other._address;
    this->_bodyData = other._bodyData;
    this->_headersData = other._headersData;
    
    for (int i = 0; i < MAX_REQUEST_SIZE + 1; i++) {
        this->_requestData[i] = other._requestData[i];
    }

    if (other._Request) {
        this->_Request = new Request(*other._Request);  // Assuming Request has a copy constructor
    } else {
        this->_Request = NULL;
    }
    if (other._Response) {
        this->_Response = new Response(*other._Response);  // Assuming Response has a copy constructor
    } else {
        this->_Response = NULL;
    }
}

sockaddr_storage        &Client::getAddr()                          {return this->_address;};
socklen_t               &Client::getAddrLen()                       {return this->_address_length;};
SOCKET                  &Client::getSocketFd()                      {return this->_socket;};
Request                 *Client::getRequest()                       {return this->_Request;}
Response                *Client::getResponse()                      {return this->_Response;}
std::string             Client::getRequestData()                   {return this->_requestData;}
int                     &Client::getRecvBytes()                     {return this->_received;}
Server *                Client::getServer()                         {return this->_server;}
std::string             Client::getHeadersData()                   {return this->_headersData;}
std::string             Client::getBodyData()                      {return this->_bodyData;}


void                    Client::setHeadersData(std::string headersData)   {this->_headersData = headersData;}
void                    Client::setBodyData(std::string bodyData)         {this->_bodyData = bodyData;}
void                    Client::setAddr(sockaddr_storage addr)      {this->_address = addr;}
void                    Client::setAddrLen(socklen_t len)           {this->_address_length = len;}
void                    Client::setSocketFd(SOCKET fd)              {this->_socket = fd;}
void                    Client::set_Request(class Request *_Request){this->_Request = _Request;}
void                    Client::set_Response(class Response *_Response){this->_Response = _Response;} 
void                    Client::setRequestData(std::string requestData)   {this->_requestData = requestData;}
void                    Client::setRecvData(int bytes)              {this->_received = bytes;}
void                    Client::setServer(Server *server)           {this->_server = server;}

Client::Client() {
  this->_Request = NULL;
  this->_Response = new Response();
  this->_server = NULL;
  memset(&this->_address, 0, sizeof(this->_address));
  this->_address_length = sizeof(this->_address);
  this->_socket = 0;
  this->_requestData = "";
  this->_headersData = "";
  this->_bodyData = "";
  this->_received = 0;
};

Client::Client(SOCKET fd) {
  this->_Request = NULL;
  this->_Response = new Response();
  this->_server = NULL;

  memset(&this->_address, 0, sizeof(this->_address));
  this->_address_length = sizeof(this->_address);
  this->_socket = fd;
  this->_requestData = "";
  this->_headersData = "";
  this->_bodyData = "";
  this->_received = 0;
};

Client::~Client(){
  std::cout << "Client destroyed" << std::endl;
  if(this->_Request)
    delete this->_Request;
  if(this->_Response)
    delete this->_Response;
};