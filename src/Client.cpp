#include "Client.hpp"

sockaddr_storage        &Client::get_addr()                          {return this->address;};
socklen_t               &Client::get_addr_len()                      {return this->address_length;};
SOCKET                  &Client::get_socket_fd()                     {return this->socket;};
char                    *Client::get_request()                       {return this->request;}
int                     &Client::get_recv_bytes()                    {return this->received;}


void                    Client::set_addr(sockaddr_storage addr)     {this->address = addr;}
void                    Client::set_addr_len(socklen_t len)         {this->address_length = len;}
void                    Client::set_socket_fd(SOCKET fd)            {this->socket = fd;}

void                    Client::set_request(char *request)          {
                                                                    strncpy(this->request, request, sizeof(this->request) - 1);
                                                                    this->request[sizeof(this->request) - 1] = '\0';
}
void                    Client::set_recv_bytes(int bytes)           {this->received = bytes;}
void                    Client::add_recv_bytes(int bytes)           {this->received += bytes;}





Client::Client() {
  memset(&this->address, 0, sizeof(this->address));
  this->address_length = sizeof(this->address);
  this->received = 0;
};

Client::~Client(){};