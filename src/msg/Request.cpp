#include "Request.hpp"

Request::Request() : _method(""), _url(""), _version(""), _body(""), _headers(){};
Request::Request(Request& other) {
    this->_method = other._method;
    this->_url = other._url;
    this->_version = other._version;
    this->_body = other._body;

    this->_headers = other._headers;
}
Request::~Request(){};

std::string &Request::getMethod(){return _method;}

std::string &Request::getUrl(){return _url;}

std::string &Request::getVersion(){return _version;}

std::map<std::string, std::string> &Request::getHeaders(){return _headers;}

std::string &Request::getBody(){return _body;}

void Request::setMethod(std::string& method) {this->_method = method;}

void Request::setUrl(std::string& url) {    this->_url = url;}

void Request::setVersion(std::string& version) {this->_version = version;}

void Request::setHeaders(std::map<std::string, std::string>& headers) {this->_headers = headers;}

void Request::setBody(std::string& body) {this->_body = body;}




