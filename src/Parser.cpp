#include "Parser.hpp"
#include "Request.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Utils.hpp"

//TODO understand how to handle errors in extract function
Request* Parser::extract(std::string headerData, std::string bodyData, Client *client) {
    
    Response *response = client->getResponse();
    Request *request = new Request();
    
    std::string line;

    std::istringstream headerStream(headerData);
    std::istringstream bodyStream(bodyData);

    std::string dataStr(headerData + bodyData);

    if (dataStr.find("\r\n\r\n") == std::string::npos) {
        response->setStatusCode(400);
        return NULL;
    }

    if(!bodyData.empty())
        request->setHasBody(true);
   
    if (std::getline(headerStream, line) && this->extractFirstLine(request, response, line) != SUCCESS){
        response->setStatusCode(400);
        return NULL;
    }   
    
    this->extractHeaders(request, headerStream);

    if(this->extractBody(request, bodyStream) != SUCCESS){
        response->setStatusCode(400);
        return NULL;
    }
    
    return request;
}





void Parser::validateResource(Client *client, Server *server)
{

    int fileType;
    std::string fileContent;

    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response)
        return;

    //differentiate between a GET AND POST DELETE request

    //TODO atm is hardcoded to the root directory
    //get the full path of the requested resource
    std::string filePath = server->getCwd() +  server->getRoot() + request->getUrl();
    
    //std::cout << "Requested file: " << filePath << std::endl;

    //check if the requested resource is accessible
    fileType = this->checkResource(filePath, response);
    if(fileType == FAILURE){
        std::cout << "Error: " << response->getStatus() << std::endl;
        response->setBody(response->getErrorPage(response->getStatus()));
        return;
    }
    
    if(S_ISDIR(fileType)){
        if(*(filePath.rbegin()) != '/'){
            std::string newUrl = request->getUrl() + "/";
            request->setUrl(newUrl);
        }
        std::string dirBody = fromDIRtoHTML(filePath, request->getUrl());
        //TODO implement the directory listing
        if(dirBody.empty()){
            response->setStatusCode(500);
            return;
        }
        response->setBody(dirBody);
        return;
    } else {
        //read the content of the requested resource
        fileContent = this->readFile(filePath, response);
    }
   
    //final check to see if there has been an error
    if(response->getStatus() != 200){
        response->setBody(response->getErrorPage(response->getStatus()));
        return;
    }

    response->setBody(fileContent);

    return ;
}


Parser::Parser() {

    this->_allowd_versions.insert("HTTP/1.1");
    this->_allowd_versions.insert("undefined");

    
    this->_implemnted_methods.insert("GET");
    this->_implemnted_methods.insert("POST");
    this->_implemnted_methods.insert("DELETE");


    this->_allowd_methods.insert("GET");
    this->_allowd_methods.insert("POST");
    this->_allowd_methods.insert("DELETE");
    
    //this->_allowd_methods.insert("PUT");
    //this->_allowd_methods.insert("HEAD");
    this->_allowd_headers.insert("content-length");
    this->_allowd_headers.insert("content-type");
    this->_allowd_headers.insert("date");
    this->_allowd_headers.insert("connection");
    this->_allowd_headers.insert("host");
    this->_allowd_headers.insert("accept");
    this->_allowd_headers.insert("accept-language");
    this->_allowd_headers.insert("alt-used");
    this->_allowd_headers.insert("accept-encoding");
    this->_allowd_headers.insert("from");
    this->_allowd_headers.insert("user-agent");
}

Parser::~Parser(){}