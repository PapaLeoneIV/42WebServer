#include "Parser.hpp"
#include "Request.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Utils.hpp"

//TODO understand how to handle errors in decompose function
Request* Parser::decompose(std::string headerData, std::string bodyData, Client *client) {
    Request *tmpRequest = new Request();
    
    std::string line;
    std::string body;
    (void)client;
    std::istringstream headerStream(headerData);

    //check if the request has a body
    if(!bodyData.empty()) {tmpRequest->setHasBody(true);}
   
    if (std::getline(headerStream, line)) {
        this->decomposeFirstLine(tmpRequest, line );
    }

    this->decomposeHeaders(tmpRequest, headerData);

    this->decomposeBody(tmpRequest, bodyData);
    
    //std::cout << tmpRequest->getBody().size() << std::endl;

    return tmpRequest;
}

ERROR Parser::parse(Request *request, Client *client) {
    Response *response = client->getResponse();
    
    //request->print();

    if(!request || !response)
        return INVALID_REQUEST;

    //request->print();
    std::string dataStr(client->getHeadersData() + client->getBodyData());
    //CLRF not found
    if (dataStr.find("\r\n\r\n") == std::string::npos) {
        response->setStatusCode(400);
        return INVALID_HEADER;
    }
    //check if the method is implemented, consult section 5.1.1 of RFC 2616
    switch (this->_allowd_methods.count(request->getMethod())) {
        case true:
            if (this->_implemnted_methods.find(request->getMethod()) == this->_implemnted_methods.end()) {
                // not permitted
                response->setStatusCode(405);
                response->setHeaders("Allow", "GET, POST, DELETE");
                return INVALID_HEADER;
            }
            break;
        case false:
                // not implemented
                response->setStatusCode(501);
                response->setHeaders("Allow", "GET, POST, DELETE");
                return INVALID_HEADER;
        }

    //only allowing http/1.1 at the moment
    if (this->_allowd_versions.find(request->getVersion()) == this->_allowd_versions.end()) {
        response->setStatusCode(505);
        return INVALID_HEADER;
    }

    //TODO check section 3.2 of RFC 2616 for the correct format of the URL
    std::string url = removeHexChars(request->getUrl());

    if (url.find_first_not_of(ALLOWED_CHARS) != std::string::npos || url.find_first_of("/") != 0) {
        response->setStatusCode(400);
        return INVALID_HEADER; 
    }

    //TODO more checks on headers
    //for reference check 9.1.1  Safe Methods of RFC 2616
    //and sectiion 4.4
    if(request->getMethod() == "GET" || request->getMethod() == "DELETE") {
        //i can set it to null and ignore the body 
        std::string empty = "";
        request->setBody(empty);
    } else if (request->getMethod() == "POST") {
        std::cout << "POST request" << std::endl;
        std::cout << "Request body: " << request->getBody().length()<< std::endl;

        if (request->getHeaders().find("content-length") != request->getHeaders().end()) {
            std::string contentLen = request->getHeaders().find("content-length")->second;
            if (request->getBody().size() != static_cast<size_t>(strToInt(contentLen))) {
                response->setStatusCode(400);
                return INVALID_HEADER;
            }
        } else if(request->getHeaders().find("transfer-encoding") != request->getHeaders().end()) {
            if (request->getHeaders().find("transfer-encoding")->second == "chunked") {
                //TODO handle it in the future
            }
        } else {
            //missing content-length and transfer encoding
            response->setStatusCode(411);
            return INVALID_HEADER;
        }
    }
    return SUCCESS;
}



void Parser::validateResource(Client *client, Server *server)
{

    int fileType;
    std::string fileContent;

    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response)
        return;

    //differentiate between a GET AND POST request

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