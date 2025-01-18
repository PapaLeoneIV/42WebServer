#include "Response.hpp"
#include "Utils.hpp"


//Response  Section 6
// CRLF = "\r\n"
// Response      =  Status-Line               ; Section 6.1
//                  *(( general-header        ; Section 4.5
//                   | response-header        ; Section 6.2
//                   | entity-header ) CRLF)  ; Section 7.1
//                  CRLF
//                  [ message-body ]          ; Section 7.2

void Response::print(){
    std::cout << "Response: " << std::endl;
    std::cout << this->_finalResponse << std::endl;
}

void Response::prepareResponse(){
    
    this->fillStatusLine();
    this->_finalResponse.append("\r\n");

    std::map<std::string, std::string>::iterator headerMapIt;
    
    for(headerMapIt = this->_headers.begin(); headerMapIt != this->_headers.end(); headerMapIt++){
        this->fillHeader(headerMapIt->first, headerMapIt->second);
    }
    this->_finalResponse.append("\r\n");

    
    this->_finalResponse.append(this->_body);
}

void Response::fillStatusLine(){
    this->_finalResponse.append("HTTP/1.1 ");
    this->_finalResponse.append(intToStr(this->_status));
    this->_finalResponse.append(" ");
    this->_finalResponse.append(getMessageFromStatusCode(this->_status));
}

void Response::fillHeader(std::string headerKey, std::string headerValue){
    this->_finalResponse.append(headerKey);
    this->_finalResponse.append(": ");
    this->_finalResponse.append(headerValue);
    this->_finalResponse.append("\r\n");    
}


std::string Response::getErrorPage(int status) {
    switch (status) {
        case 400: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/400.html");
        case 403: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/403.html");
        case 404: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/404.html");
        case 405: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/405.html");
        case 411: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/411.html");
        case 500: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/500.html");
        case 501: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/501.html");
        case 505: return readTextFile("/nfs/homes/rileone/42WebServer/static/errorPage/505.html");
    }
    return "";
}



std::string &Response::getResponse()                                    {return this->_finalResponse;}
std::string &Response::getContentType()                                 {return this->_content_type;};
std::string &Response::getBody()                                        {return this->_body;}
std::string &Response::getStatusMessage()                               {return this->_status_message;}
std::map<std::string, std::string> &Response::getHeaders()              {return this->_headers;}

int &Response::getStatus()                                              {return this->_status;};

void Response::setResponse(char *response)                              {this->_finalResponse = response;}
void Response::setBody(std::string body)                                {this->_body = body;}
void Response::setContentType(std::string content_type)                 {this->_content_type = content_type;}
void Response::setStatusMessage(std::string status_message)             {this->_status_message = status_message;}
void Response::setStatusCode(int status)                                {this->_status = status;}
void Response::setHeaders(std::string key, std::string value) {
    std::pair<std::string, std::string> KValHead(key, value);
    this->_headers.insert(KValHead);
}



Response::Response()
{
    this->_finalResponse = "";
    this->_body = "";
    this->_content_type = "";
    this->_status_message = "OK";
    this->_status = 200;
}

Response::Response(int status, const char *status_message)
{
    this->_finalResponse = "";
    this->_body = "";
    this->_content_type = "";
    this->_status_message = status_message;
    this->_status = status;
}


Response::Response(int status, std::string status_message)
{
  this->_finalResponse = "";
    this->_status = status;
    this->_status_message = status_message;
    this->_body = "";
    this->_content_type = "";
}

Response::~Response()
{}