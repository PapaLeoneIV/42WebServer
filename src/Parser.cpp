#include "Parser.hpp"

Parser::Parser() {
    this->_allowd_methods.push_back("GET");
    this->_allowd_methods.push_back("POST");

    this->_allowd_versions.push_back("HTTP/1.1");

    this->_mandatory_headers.push_back("host");
    
    this->_allowd_headers.push_back("accept");
    this->_allowd_headers.push_back("content-length");
    this->_allowd_headers.push_back("content-type");
    this->_allowd_headers.push_back("accept-language");
    this->_allowd_headers.push_back("alt-used");
    this->_allowd_headers.push_back("accept-encoding");
    this->_allowd_headers.push_back("date");
    this->_allowd_headers.push_back("from");
    this->_allowd_headers.push_back("user-agent");
}

Parser::~Parser(){}

ERROR Parser::checkMaxRecvBytes(int recv_bytes){
    if(recv_bytes >= MAX_REQUEST_SIZE){
        std::cerr << "Error: request too long" << std::endl;
        return INVALID_REQUEST_SIZE;
    }
    return 0;
}

ERROR Parser::checkRecvBytes(int recv_bytes){

        if(recv_bytes == -1){
            std::cerr << "Error: recv failed: closing connection" << std::endl;
            return INTERNAL_RECV_ERROR;
        }
        if(recv_bytes == 0){
            std::cerr << "Error: connection closed by client" << std::endl;
            return INVALID_CONNECTION_CLOSE_BY_CLIENT;
        }
        return 0;
}


Request* Parser::parse(std::vector<Client*>::iterator client) {
    Request *request = new Request();
    std::istringstream iss((*client)->getRequest());

    std::string url;
    std::string version;
    std::string method;

    std::string line;
    std::string body;

    if (std::getline(iss, line)) {
        std::istringstream firstLineStream(line);
        firstLineStream >> method >> url >> version;

        request->setMethod(method);
        request->setUrl(url);
        request->setVersion(version);
    }

    std::map<std::string, std::string> headers;

    while (std::getline(iss, line) && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            headerName = headerName.substr(headerName.find_first_not_of(" \t"));
            headerValue = headerValue.substr(headerValue.find_first_not_of(" \t"));

            headerName = to_lowercase(headerName);

            headers[headerName] = headerValue;
        }
    }

    request->setHeaders(headers);

    std::getline(iss, body, '\0');
    request->setBody(body);

    if(this->validate(request) != 0){
        
    }

    return request;
}

ERROR Parser::checkMethod(std::string method) {
    std::vector<std::string>::iterator it;
    for (it = this->_allowd_methods.begin(); it != this->_allowd_methods.end(); ++it) {
        if (method == *it) {
            return 0; 
        }
    }
    return INVALID_METHOD;
}

ERROR Parser::checkUrl(std::string url) {
    if (url.empty() || url[0] != '/') {
        return INVALID_URL; 
    }
    return 0; 
}

ERROR Parser::checkVersion(std::string version) {
    std::vector<std::string>::iterator it;
    for (it = this->_allowd_versions.begin(); it != this->_allowd_versions.end(); ++it) {
        if (version != *it) {
            return INVALID_VERSION; 
        }
    }
    return 0; 
}

ERROR Parser::checkHeaders(std::map<std::string, std::string> req_headers) {
    std::vector<std::string>::iterator mand_headers_it;
    for (mand_headers_it = this->_mandatory_headers.begin(); mand_headers_it != this->_mandatory_headers.end(); ++mand_headers_it) {
        if (req_headers.find(*mand_headers_it) == req_headers.end() || req_headers[*mand_headers_it].empty()) {
            return INVALID_HEADER; 
        }
    }
    return 0; 
}

ERROR Parser::checkBody(std::string body, Request *request) {
    (void)request;
    (void)body; 
    return 0;
}

ERROR Parser::validate(Request *request) {
    ERROR error;
    if ((error = checkMethod(request->getMethod()))) return error;
    if ((error = checkUrl(request->getUrl()))) return error;
    if ((error = checkVersion(request->getVersion()))) return error;
    if ((error = checkHeaders(request->getHeaders()))) return error;
    if ((error = checkBody(request->getBody(), request))) return error;

    return 0;
}
