#include "Parser.hpp"

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


void Parser::validateResource(Client *client, Server *server)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    std::string fullPath = (std::string(getcwd(NULL, 0)) +  server->getRoot() + request->getUrl());
    struct stat sb;

    if(access(fullPath.c_str(), F_OK) == -1){
        response->setStatusCode(404);
        response->setStatusMessage("Not Found");
        return ;
    }

    if(stat(fullPath.c_str(), &sb) == -1){
        response->setStatusCode(500);
        response->setStatusMessage("Internal Server Error");
        return ;
    }


    if(S_ISREG(sb.st_mode)){
        if(checkPermissions(fullPath, R_OK) != SUCCESS){
            response->setStatusCode(403);
            response->setStatusMessage("Forbidden: Permission Denied: Read access denied");
            return ;
        }
    }
    else if(S_ISDIR(sb.st_mode)){
        if(checkPermissions(fullPath, R_OK) == SUCCESS){
            response->setStatusCode(403);
            response->setStatusMessage("Forbidden: Permission Denied: Read access denied");
            return ;
        }
        //TODO check if index.html exists
        //TODO check if the directory is readable
        //TODO check if the directory is executable
    }else{
        response->setStatusCode(403);
        response->setStatusMessage("Forbidden: Permission Denied: Not a regular file or directory");
        return ;
    }
    

    //check if is it accessible
    if(access(fullPath.c_str(), F_OK) == -1){
        response->setStatusCode(404);
        response->setStatusMessage("Not Found");
        return ;
    }
    //check if is it readable
    if(access(fullPath.c_str(), R_OK) == -1){
        response->setStatusCode(403);
        response->setStatusMessage("Forbidden: Permission Denied: Read access denied");
        return ;
    }

    return ;
}

void Parser::parse(Request *request, Client *client) {

    std::string dataStr(client->getRequestData());
    //CLRF not found
    if (!dataStr.find("\r\n\r\n")) {
        client->getResponse()->setStatusCode(400);
        client->getResponse()->setStatusMessage("Bad Request: CLRF not found");
        return ;
    }
    //check if the method is implemented, consult section 5.1.1 of RFC 2616
    switch (this->_allowd_methods.count(request->getMethod())) {
        case true:
            if (this->_implemnted_methods.find(request->getMethod()) == this->_implemnted_methods.end()) {
                // not permitted
                client->getResponse()->setStatusCode(405);
                client->getResponse()->setStatusMessage("Method Not Allowed");
                client->getResponse()->fillHeader("Allow", "GET, POST, DELETE");
                return;
            }
            break;
        case false:
                // not implemented
                client->getResponse()->setStatusCode(501);
                client->getResponse()->setStatusMessage("Not Implemented");
                client->getResponse()->fillHeader("Allow", "GET, POST, DELETE");
                return;
        }

    //only allowing http/1.1 at the moment
    if (this->_allowd_versions.find(request->getVersion()) == this->_allowd_versions.end()) {
        client->getResponse()->setStatusCode(505);
        client->getResponse()->setStatusMessage("HTTP Version Not Supported");
        return;
    }

    //TODO check section 3.2 of RFC 2616 for the correct format of the URL
    request->print();
    std::string url = analyzeUrl(request->getUrl());

    if (url.find_first_not_of(ALLOWED_CHARS) != std::string::npos || url.find_first_of("/") != 0) {
        client->getResponse()->setStatusCode(400);
        client->getResponse()->setStatusMessage("Bad Request: Invalid URL");
        return; 
    }

    //TODO more checks on headers
    //for reference check 9.1.1  Safe Methods of RFC 2616
    //and sectiion 4.4
    if(request->getMethod() == "GET" || request->getMethod() == "DELETE" /* || request->getMethod() == "HEAD" */) {
        //i can set it to null and ignore the body 
        std::string empty = "";
        request->setBody(empty);
 
    } else if (request->getMethod() == "POST"/*  || request->getMethod() == "PUT" */) {
        if (request->getHeaders().find("content-length") != request->getHeaders().end()) {
        std::string contentLen = request->getHeaders().find("content-length")->second;
        if (request->getBody().size() != static_cast<size_t>(strToInt(contentLen))) {
            client->getResponse()->setStatusCode(400);
            client->getResponse()->setStatusMessage("Bad Request: Content-Length does not match the body size");
            return;
        }
        } else if(request->getHeaders().find("transfer-encoding") != request->getHeaders().end()) {
            if (request->getHeaders().find("transfer-encoding")->second == "chunked") {
                //TODO handle it in the future
            }
        } else {
            //missing content-length and transfer encoding
            client->getResponse()->setStatusCode(411);
            client->getResponse()->setStatusMessage("Length Required");
            return;
        }
    }
}

Request* Parser::decompose(char *data) {
    std::string url, version, method;
    std::string line, body;

    std::istringstream iss(data);

    Request *tmpRequest = new Request();

    if (std::getline(iss, line)) {
        std::istringstream firstLineStream(line);
        firstLineStream >> method >> url >> version;
    }
    
    tmpRequest->setMethod(method);
    //if we find % we should parse the next two char as HEX and replace it with the actual char
    url = analyzeUrl(url);

    tmpRequest->setUrl(url);
    tmpRequest->setVersion(version);

    std::map<std::string, std::string> headers;


    //TODO update check on CLRF
    while (std::getline(iss, line) && line != "\r\n" && !line.empty()) {   
        size_t colonPos = line.find(':');

        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            // Remove all carriage return, space, tabs from key and value
            headerName.erase(std::remove(headerName.begin(), headerName.end(), '\r'), headerName.end());
            headerName.erase(std::remove(headerName.begin(), headerName.end(), ' '), headerName.end());
            headerName.erase(std::remove(headerName.begin(), headerName.end(), '\t'), headerName.end());
            headerValue.erase(std::remove(headerValue.begin(), headerValue.end(), '\r'), headerValue.end());
            headerValue.erase(std::remove(headerValue.begin(), headerValue.end(), ' '), headerValue.end());
            headerValue.erase(std::remove(headerValue.begin(), headerValue.end(), '\t'), headerValue.end());

            // Case insensitive
            std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

            headers[headerName] = headerValue;
        }
    }

    tmpRequest->setHeaders(headers);

    std::getline(iss, body, '\0');
    
    tmpRequest->setBody(body);


    return tmpRequest;
}