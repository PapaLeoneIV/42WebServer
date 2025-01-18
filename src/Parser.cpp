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

int Parser::checkResource(std::string filePath, Response* response) {
    struct stat sb;

    if (access(filePath.c_str(), F_OK) == FAILURE) {
        response->setStatusCode(404);
        return FAILURE;
    }

    if (stat(filePath.c_str(), &sb) == FAILURE) {
        response->setStatusCode(500);
        return FAILURE;
    }

    if (S_ISREG(sb.st_mode) || S_ISDIR(sb.st_mode)) {
        if (checkPermissions(filePath, R_OK) != SUCCESS) {
            response->setStatusCode(403);
            return FAILURE;
        }
    } else {
        response->setStatusCode(403);
        return FAILURE;
    }
    return sb.st_mode;
}


void Parser::validateResource(Client *client, Server *server)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response)
        return;

    int fileType;
    std::string fileContent;

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

ERROR Parser::parse(Request *request, Client *client) {
    Response *response = client->getResponse();
    
    if(!request || !response)
        return INVALID_REQUEST;

    request->print();
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
    std::string url = analyzeUrl(request->getUrl());

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

Request* Parser::decompose(std::string data) {
    std::string url, version, method;
    std::string line, body;
    std::istringstream iss(data);
    
    Request *tmpRequest = new Request();

    //check if the request has a body
    if(!data.substr(data.find("\r\n\r\n") + 4).empty()) {tmpRequest->setHasBody(true);}
   
    
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
            for (size_t i = 0; i < headerName.size(); ++i) {
                if (headerName[i] == '\r' || headerName[i] == ' ' || headerName[i] == '\t') {
                    headerName.erase(i, 1);
                    --i;
                }
            }

            for (size_t i = 0; i < headerValue.size(); ++i) {
                if (headerValue[i] == '\r' || headerValue[i] == ' ' || headerValue[i] == '\t') {
                    headerValue.erase(i, 1);
                    --i;
                }
            }

            // Case insensitive
            std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

            headers[headerName] = headerValue;
        }
    }
    tmpRequest->setHeaders(headers);


    tmpRequest->printHeaders();
    //After we got the headers from the request, we should check what type of request is GET, POST, DELETE 
    //and if it has a body parse it accordingly reference blog(https://http.dev/post)
    if(tmpRequest->getMethod() == "POST" && tmpRequest->hasBody()) {
        
        //mi prendo il value che segue l header "content type"
        std::string contType = tmpRequest->getHeaders().find("content-disposition")->second;
        std::cout << "ContType: " << contType << std::endl;
        if(contType == "text/plain"){

            std::getline(iss, body, '\0');
            tmpRequest->setBody(body);
        
        } else if(contType.find(";") != std::string::npos && contType.substr(0, contType.find(";")) == "multipart/form-data") {
            //TODO understand how to handle errors in decompose function 
            //parse the body of the request
            std::string boundary = contType.substr(contType.find("boundary=") + 9);
            this->parseMultipart(iss, boundary);
        }
        else{
            //TODO handle other content types
            
        }
            
    }

    tmpRequest->setBody(body);


    return tmpRequest;
}
 //i should check what content type it has to decide how to parse it
        /*
         * 2-> multipart/form-data; boundary=”NextField”
         *      POST / HTTP/1.1
         *       Host: www.example.re
         *       Content-Type: multipart/form-data; boundary=”NextField”
         *       Content-Length: 125
         *       
         *       --NextField
         *       Content-Disposition: form-data; name=”Job”
         *       
         *       100
         *       
         *       -- NextField
         *       Content-Disposition: form-data; name=”Priority”
         *       
         *       2
         * 3-> text/plain
         *      POST /help.txt HTTP/1.1
         *       Host: www.example.re
         *       Content-Type: text/plain
         *       Content-Length: 51
         *
         *       Please visit www.example.re for the latest updates!
         *
         * 
         * 
         */


std::string joinContentBetweenBoundaries(std::istringstream &iss, const std::string &boundary) {
    std::string line;
    std::string content;
    bool withinBoundary = false;

    while (std::getline(iss, line)) {
        if (line == "--" + boundary + "\r") {
            withinBoundary = true;
            continue;
        }
        if (withinBoundary && line == "--" + boundary + "--\r") {
            withinBoundary = false;
            break;
        }
        if (withinBoundary) {
            content += line + "\n";
        }
    }

    if (!content.empty() && *content.rbegin() == '\n') {
        content.erase(content.size() - 1);
    }

    return content;
}


std::vector<std::string> splitByContentDisposition(std::istringstream &iss) {
    std::vector<std::string> sections;
    std::string line;
    std::string currentSection;

    while (std::getline(iss, line)) {
        if (line.find("Content-Disposition: form-data;") != std::string::npos) {
            if (!currentSection.empty()) {
                sections.push_back(currentSection);
                currentSection = ""; 
            }
        }
        currentSection += line + "\n";
    }
    if (!currentSection.empty()) {
        sections.push_back(currentSection);
    }

    return sections;
}

std::map<std::string, std::string> parseSection(const std::string &section) {
    std::map<std::string, std::string> extractedData;
    std::istringstream sectionStream(section);
    std::string line;
    bool isBody = false;
    std::string body;

    while (std::getline(sectionStream, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line = line.substr(0, line.size() - 1);
        }

        if (isBody) {
            body += line + "\n";
        } else if (line.find("Content-Disposition:") != std::string::npos) {
            std::string::size_type namePos = line.find("name=\"");
            if (namePos != std::string::npos) {
                namePos += 6; // Skip 'name="'
                std::string::size_type endPos = line.find("\"", namePos);
                if (endPos != std::string::npos) {
                    extractedData["name"] = line.substr(namePos, endPos - namePos);
                }
            }

            std::string::size_type filenamePos = line.find("filename=\"");
            if (filenamePos != std::string::npos) {
                filenamePos += 10; // Skip 'filename="'
                std::string::size_type endPos = line.find("\"", filenamePos);
                if (endPos != std::string::npos) {
                    extractedData["filename"] = line.substr(filenamePos, endPos - filenamePos);
                }
            }
        } else if (line.find("Content-Type:") != std::string::npos) {
            extractedData["contentType"] = line.substr(14); // Skip 'Content-Type: '
        } else if (line.empty()) {
            isBody = true;
        }
    }

    // Remove trailing newline from the body
    if (!body.empty() && body[body.size() - 1] == '\n') {
        body = body.substr(0, body.size() - 1);
    }

    extractedData["body"] = body;

    return extractedData;
}

void Parser::parseMultipart(std::istringstream &formDataStream, std::string boundary) {
     // Parse the multipart form data                                                                                                                                                                               
    std::istringstream iss(formDataStream.str());
    std::string sections = joinContentBetweenBoundaries(iss, boundary);
    std::istringstream sectionsStream(sections);
    std::vector<std::string> sezioni = splitByContentDisposition(sectionsStream);
    for (std::vector<std::string>::const_iterator it = sezioni.begin(); it != sezioni.end(); ++it) {
        std::map<std::string, std::string> extractedData = parseSection(*it);

    }

}

