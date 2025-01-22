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


std::string Parser::readFile(std::string filePath, Response *response)
{
    std::string fileContent;
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        response->setStatusCode(404);
        return "";
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    for (std::vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it) {
        fileContent.push_back(*it);
    }

    return fileContent;
}

void Parser::checkAccessability(std::string filePath, Response* response) {
    struct stat sb;

    if (access(filePath.c_str(), F_OK) == -1) {
        response->setStatusCode(404);
        return;
    }

    if (stat(filePath.c_str(), &sb) == -1) {
        response->setStatusCode(500);
        return;
    }

    if (S_ISREG(sb.st_mode) || S_ISDIR(sb.st_mode)) {
        if (checkPermissions(filePath, R_OK) != SUCCESS) {
            response->setStatusCode(403);
        }
    } else {
        response->setStatusCode(403);
    }
}


void Parser::validateResource(Client *client, Server *server)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response)
        return;

    //get the full path of the requested resource
    std::string filePath = server->getCwd() +  server->getRoot() + request->getUrl();

    //check if the requested resource is accessible
    this->checkAccessability(filePath, response);
   
   //read the content of the requested resource
    std::string fileContent = this->readFile(filePath, response);
   
    //final check to see if there has been an error
    if(response->getStatus() != 200)
    {
        response->setBody(response->getErrorPage(response->getStatus()));
        return;
    }

    response->setBody(fileContent);

    return ;
}

ERROR Parser::parse(Request *request, Client *client) {
    std::string dataStr(client->getRequestData());
    //CLRF not found
    if (!dataStr.find("\r\n\r\n")) {
        client->getResponse()->setStatusCode(400);
        return INVALID_HEADER;
    }
    //check if the method is implemented, consult section 5.1.1 of RFC 2616
    switch (this->_allowd_methods.count(request->getMethod())) {
        case true:
            if (this->_implemnted_methods.find(request->getMethod()) == this->_implemnted_methods.end()) {
                // not permitted
                client->getResponse()->setStatusCode(405);
                client->getResponse()->setHeaders("Allow", "GET, POST, DELETE");
                return INVALID_HEADER;
            }
            break;
        case false:
                // not implemented
                client->getResponse()->setStatusCode(501);
                client->getResponse()->setHeaders("Allow", "GET, POST, DELETE");
                return INVALID_HEADER;
        }

    //only allowing http/1.1 at the moment
    if (this->_allowd_versions.find(request->getVersion()) == this->_allowd_versions.end()) {
        client->getResponse()->setStatusCode(505);
        return INVALID_HEADER;
    }

    // TODO: check section 3.2 of RFC 2616 for the correct format of the URL
    std::string url = analyzeUrl(request->getUrl());
    // TODO: test this
    if (url.find_first_not_of(ALLOWED_CHARS) != std::string::npos || url.find_first_of("/") != 0) {
        client->getResponse()->setStatusCode(400);
        return INVALID_HEADER; 
    }

    // TODO: more checks on headers
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
                return INVALID_HEADER;
            }
        } else if(request->getHeaders().find("transfer-encoding") != request->getHeaders().end()) {
            if (request->getHeaders().find("transfer-encoding")->second == "chunked") {
                //TODO handle it in the future
            }
        } else {
            //missing content-length and transfer encoding
            client->getResponse()->setStatusCode(411);
            return INVALID_HEADER;
        }
    }
    return SUCCESS;
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