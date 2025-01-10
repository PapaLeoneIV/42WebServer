#include "Parser.hpp"

Parser::Parser() {

    this->_allowd_versions.insert("HTTP/1.1");
    
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

void Parser::parse(Request *request, Client *client) {
    
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
    // http_URL = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]    
    if (this->isValidUrl(request->getUrl()) == false) {
        client->getResponse()->setStatusCode(400);
        client->getResponse()->setStatusMessage("Bad Request");
        return; 
    }

    //TODO more checks on headers
    //for reference check 9.1.1  Safe Methods of RFC 2616
    //and sectiion 4.4
    if(request->getMethod() == "GET" || request->getMethod() == "DELETE" /* || request->getMethod() == "HEAD" */) {
        //i can set it to null  and ignore the body 
        std::string empty = "";
        request->setBody(empty);


        
    } else if (request->getMethod() == "POST"/*  || request->getMethod() == "PUT" */) {
        if (request->getHeaders().find("content-length") != request->getHeaders().end()) {
        std::string contentLen = request->getHeaders().find("content-length")->second;
        if (request->getBody().size() != static_cast<size_t>(strToInt(contentLen))) {
            client->getResponse()->setStatusCode(400);
            client->getResponse()->setStatusMessage("Bad Request");
            return;
        }
        }
        else if(request->getHeaders().find("transfer-encoding") != request->getHeaders().end()) {
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
    tmpRequest->setUrl(url);
    tmpRequest->setVersion(version);

    std::map<std::string, std::string> headers;

    while (std::getline(iss, line) && line[0] != '\r' && line[1] != '\n') {   
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            headerName = headerName.substr(headerName.find_first_not_of(" \t"));
            headerValue = headerValue.substr(headerValue.find_first_not_of(" \t"));

            //case insensitive
            headerName = to_lowercase(headerName);

            headers[headerName] = headerValue;
        }
    }

    tmpRequest->setHeaders(headers);

    std::getline(iss, body, '\0');
    
    tmpRequest->setBody(body);

    std::cout << "print body: " << body << std::endl; 

    return tmpRequest;
}

bool Parser::isValidUrl(std::string &url) {
    (void)url;
    return true;
}
