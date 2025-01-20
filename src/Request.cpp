#include "Request.hpp"

Request::Request() : _method(""), _url(""), _version(""), _body(""), _headers(), _contType(""), _hasBody(false) {};


int         &Request::getContentLength()                                 {return this->_contentLength;}

std::string &Request::getContentName()                                   {return this->_contentName;}

std::string &Request::getContentFilename()                               {return this->_contentFilename;}

std::string &Request::getBoundary()                                      {return this->_boundary;}

std::string &Request::getMethod()                                       {return this->_method;}

std::string &Request::getUrl()                                          {return this->_url;}

std::string &Request::getVersion()                                      {return this->_version;}

std::map<std::string, std::string> &Request::getHeaders()               {return this->_headers;}

std::string &Request::getBody()                                         {return this->_body;}

std::string &Request::getContType()                                     {return this->_contType;}

bool &Request::hasBody()                                                 {return this->_hasBody;}

void Request::setHasBody(bool hasBody)                                  {this->_hasBody = hasBody;}

void Request::setMethod(std::string& method)                            {this->_method = method;}

void Request::setUrl(std::string& url)                                  {this->_url = url;}

void Request::setVersion(std::string& version)                          {this->_version = version;}

void Request::setHeaders(std::map<std::string, std::string>& headers)   {this->_headers = headers;}

void Request::setBody(std::string& body)                                {this->_body = body;}

void Request::setContType(std::string& contType)                        {this->_contType = contType;}

void Request::setContentLength(int contentLength)              {this->_contentLength = contentLength;}

void Request::setContentName(std::string& contentName)                  {this->_contentName = contentName;}

void Request::setContentFilename(std::string& contentFilename)          {this->_contentFilename = contentFilename;}

void Request::setBoundary(std::string& boundary)                        {this->_boundary = boundary;}


Request::Request(Request& other) {
    this->_method = other._method;
    this->_url = other._url;
    this->_version = other._version;
    this->_body = other._body;
    this->_headers = other._headers;
    this->_contType = other._contType;
    this->_hasBody = other._hasBody;
}
Request::~Request(){};

void Request::print() {
    std::cout << "Method: " << this->_method << std::endl;
    std::cout << "Url: " << this->_url << std::endl;
    std::cout << "Version: " << this->_version << std::endl;
    std::cout << "Headers: " << std::endl;
    this->printHeaders() ;  
    std::cout << "Body: " << this->_body << std::endl;
}

void Request::printHeaders() {
    std::string headers = "";
    for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); ++it) {
        headers += it->first + ": " + it->second + "\n";
        std::cout << it->first << " : " << it->second << std::endl;
    }
}