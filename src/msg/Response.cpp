#include "Response.hpp"

#include <iostream>
#include <sstream>

std::string int_to_string(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

void Response::prepare_response(std::string b){
    
    std::string header;
    this->body = b;

    if(!this->body.empty())
        this->content_length = this->body.size();

    header += "HTTP/1.1 " + int_to_string(this->status) + " " + this->status_message + "\r\n";
    header += "Content-Type: " + this->headers["Content-Type"] + "\r\n";
    header += "Content-Length: " + int_to_string(this->content_length) + "\r\n";
    header += "\r\n";

    this->response = new char[(header.size() + this->body.size()) + 1];
    response.append(header);
    response.append(this->body);

    this->response[header.size() + this->body.size()] = '\0';
}

ERROR Response::send_response(SOCKET fd)
{
    ERROR error;

    prepare_response(this->body);

    if((error = send(fd, (const void *)this->response.c_str(), this->content_length, this->flags)))
    {
        //TODO throw exception
        std::cerr << "Error: sending response failed" << std::endl;
        return error;
    }
    return 0;
}



int Response::get_content_length(){ return this->content_length;}

int Response::get_flags(){ return this->flags;}

std::string Response::get_response(){ return this->response;}

void Response::set_response(char *response) { this->response = response; }

void Response::set_content_length(int size) { this->content_length = size; }

void Response::set_flags(int flags) { this->flags = flags; }


Response::Response()
{
    this->response = "";
    this->status = 0;
    this->content_length = 0;
    this->flags = 0;
    std::vector<std::string> headers_type;
    headers_type.push_back("Content-Type");
    headers_type.push_back("Content-Length");

    for(size_t i = 0; i < headers_type.size(); i++)
    {
        if (this->headers.find(headers_type[i]) == this->headers.end())
        {
            this->headers[headers_type[i]] = "";
        }
    }   
}

Response::Response(int status, const char *status_message)
{
    this->status = status;
    this->status_message = status_message;
    this->response = "";
    this->content_length = 0;
    this->flags = 0;

    std::vector<std::string> headers_type;
    headers_type.push_back("Content-Type");
    headers_type.push_back("Content-Length");
    
    for(size_t i = 0; i < headers_type.size(); i++)
    {
        if (this->headers.find(headers_type[i]) == this->headers.end())
        {
            this->headers[headers_type[i]] = "";
        }
    }
}

Response::~Response()
{}