#include "Response.hpp"
#include "Utils.hpp"

void Response::prepareResponse(std::string b){
    
    std::string header;
    this->_body = b;

    if(!this->_body.empty())
        this->_content_length = this->_body.size();

    header += "HTTP/1.1 " + int_to_string(this->_status) + " " + this->_status_message + "\r\n";
    header += "Content-Type: text/plain\r\n";
    header += "Content-Length: " + int_to_string(this->_content_length) + "\r\n";
    header += "Connection: close\r\n";
    header += "\r\n";

    this->_response = header + this->_body;
}

std::string &Response::getResponse(){ return this->_response;}
std::string &Response::getContentType(){return this->_content_type;};
std::string &Response::getBody(){ return this->_body;}
std::string &Response::getStatusMessage(){ return this->_status_message;}
std::map<std::string, std::string> &Response::getHeaders(){ return this->_headers;}

int &Response::getStatus(){ return this->_status;};
int &Response::getContentLength(){ return this->_content_length;}
int &Response::getFlags(){ return this->_flags;}

void Response::setResponse(char *response) { this->_response = response; }
void Response::setContentLength(int size) { this->_content_length = size; }
void Response::setFlags(int flags) { this->_flags = flags; }


Response::Response()
{
    this->_response = "";
    this->_status = 0;
    this->_content_length = 0;
    this->_flags = 0;
    std::vector<std::string> headers_type;
    headers_type.push_back("Content-Type");
    headers_type.push_back("Content-Length");

    for(size_t i = 0; i < headers_type.size(); i++)
    {
        if (this->_headers.find(headers_type[i]) == this->_headers.end())
        {
            this->_headers[headers_type[i]] = "";
        }
    }   
}

Response::Response(int status, const char *status_message)
{
    this->_status = status;
    this->_status_message = status_message;
    this->_response = "";
    this->_content_length = 0;
    this->_flags = 0;

    std::vector<std::string> headers_type;
    headers_type.push_back("Content-Type");
    headers_type.push_back("Content-Length");
    
    for(size_t i = 0; i < headers_type.size(); i++)
    {
        if (this->_headers.find(headers_type[i]) == this->_headers.end())
        {
            this->_headers[headers_type[i]] = "";
        }
    }
}

Response::~Response()
{}