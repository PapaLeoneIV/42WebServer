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

void Request::setContentLength(int contentLength)                       {this->_contentLength = contentLength;}

void Request::setContentName(std::string& contentName)                  {this->_contentName = contentName;}

void Request::setContentFilename(std::string& contentFilename)          {this->_contentFilename = contentFilename;}

void Request::setBoundary(std::string& boundary)                        {this->_boundary = boundary;}




void Request::feed(std::string buffer){
    std::map<int, std::string> methods;

    methods.insert(std::make_pair(GET, "GET"));
    methods.insert(std::make_pair(POST, "POST"));
    methods.insert(std::make_pair(DELETE, "DELETE"));
    methods.insert(std::make_pair(PUT, "PUT"));
    methods.insert(std::make_pair(HEAD, "HEAD"));


    //https://datatracker.ietf.org/doc/html/rfc7230#section-3
    std::string content;
    this->state = Method_state;
    for(size_t i = 0; i < buffer.size(); i++){
        char character = buffer[i];
        switch(this->state){
            case Method_state: {
                if(character == 'G'){
                    this->_method = "GET";
                    content.append("ET");
                    i += 1;
                    this->state = Space_after_method;
                    break;
                }
                else if(character == 'P'){
                    this->state = Post_or_Put_state;
                    break;
                }
                else if (character == 'D'){
                    this->_method = "DELETE";
                    content.append("ELETE");
                    i += 4;
                    this->state = Space_after_method;
                    break;
                } else {
                    this->error = -1; //invalid method
                    return;
                }   
            }
            case Post_or_Put_state: {
                if(character == 'O'){
                    this->_method = "POST";
                    content.append("OST");
                    this->state = Space_after_method;
                    i += 2;
                    break;
                }
                else if (character == 'U')
                    this->method == "PUT";
                    content.append("UT");
                    this->state = Space_after_method;
                    i += 1;
                    break;
                }
                else{
                    this->error = -1; //invalid method
                    return;
                }
            }
            case Space_after_method: {
                if(character != ' ')
                {
                    this->error = -1; //request not valid
                    return;
                }
                this->state = Url_begin_state;
                break;
            }
            case Url_begin_state:{
                if(character != '/')
                {
                    this->error = -1; //request not valid
                    return;
                }
                this->state =  Url_string_state;
                break;
            }
            case Url_string_state:{
                // https://datatracker.ietf.org/doc/html/rfc1738#section-2.1
                if((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')  
                || (character >= '0' && character <= '9') || (character == '+') || (character == '.')
                || (character == '-') || (character == '_') || (character == '!') || (character == '$')  
                || (character == '*') || (character == '\'') || (character == '(') || (character == ')')
                || (character == '/')){
                    break;
                }
                //TODO '?' 
                if(character == '?'){
                    //TODO :
                    //switch to state "extract query params"
                }

                //% https://datatracker.ietf.org/doc/html/rfc3986#section-2.1 HTTP/1.1
                if(character == '%'){
                    // TODO :
                    //switch to state "encoded percent"
                }
                
                if(character == ' ')
                {
                    this->state = Space_after_url;
                    break;
                }
                
            }
            case Space_after_url: {
                if(character != 'H'){
                    this->error = -1; //request not valid
                    return;
                }
                this->state =  Http_version_state_H;
                break;
            }
            //"HTTP/1.1 only version admitted
            case Http_version_state_H:{
                if(character != 'T'){
                    this->error = -1;
                    return;
                }
                this->state = Http_version_state_HT;
                break;
            }
            case Http_version_state_HT:{
                if(character != 'T'){
                    this->error = -1;
                    return;
                }
                this->state = Http_version_state_HTT;
                break;
            }
            case Http_version_state_HTT:{
                if(character != 'P'){
                    this->error = -1;
                    return;
                }
                this->state = Http_version_state_HTTP;
                break;
            }
            case Http_version_state_HTTP:{
                if(character != '/'){
                    this->error = -1;
                    return;
                }
                this->state = Http_version_state_HTTP_sep;
                break;
            }
            case Http_version_state_HTTP_sep:{
                if(character != '1'){
                    this->error = -1;
                    return;
                }
                this->state = Http_version_state_HTTP_sep_one;
                break;
            }
            case Http_version_state_HTTP_sep_one:{
                if(character == '.'){
                    this->error = -1;
                    return;
                }
                this->state = Http_version_state_HTTP_sep_one_dot;
                break;
                
            }
            case Http_version_state_HTTP_sep_one_dot:{
                if(character != '1'){   
                    this->error = -1;
                    return;
                }
                this->state = Http_version_state_HTTP_sep_one_dot_one;
                break;
            }
            case Http_version_state_HTTP_sep_one_dot_one:{
                if(character != '\r'){
                    this->error = -1;
                    return;
                }
                this->state = firstline_CR;
                break;
            }
            case firstline_CR:{
                if(character != '\n'){
                    this->error = -1;
                    return;
                }
                this->state = firstline_LF;
                break;
            }
            case firstline_LF:{
                if(character == '\n'){
                    this->state = headers_state;
                    break;
                }
                else{
                    this->error = -1;
                    return;
                }
            }
            //HTTP headers are structured such that each header field consists of a case-insensitive field name followed by a colon (:),
            // optional leading whitespace, the field value, and optional trailing whitespace.They are serialized into a single string where 
            // individual header fields are separated by CRLF (carriage return and line feed, represented by \r\n in many programming languages).
            case header_start: {
                if(character == ':')
                {
                    this->state = headers_trailing_space_start;
                    break;
                }
                if(character == ' ')
                {
                    this->error = -1;
                    return;
                }
                break;
            }
            case headers_trailing_space_start: {
                if(character == ' ')
                {
                    break;
                }
                this->state = headers_value
                break;
            }
            case headers_value: {
                if(character == ' ')
                {
                    this->state = headers_trailing_space_end;
                    break;
                }
                this->state = headers_value
                break;
            
            }
            case headers_trailing_space_end: {
                if(character == ' '){
                    break;
                }
                if(character == '\r'){
                    this->state = headers_CR
                }
                this->state = headers_value
                break;
            }
            case headers_CR:{
                if(character != '\n'){
                    this->error = -1;
                    return;
                }
                this->state = 
            }
            
            content += character;
        }
    }


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