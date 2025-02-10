#include "Parser.hpp"
#include "Request.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Utils.hpp"



/**
 * Questa funzione serve ad estrarre tutte le informazioni dalla request, 
 * viene scomposta la prima linea in (METHOD, URL, VERSION), successivamente 
 * vengono insertiti gli HEADERS all interno di una mappa, ed infine il BODY
 * viene estratto in base al tipo di trasferimento presente negl headers(TEXT/PLAIN, TRANSFER-ENCODING, MULTIPART-FORMDATA).
 */
// Request* Parser::extract(std::string headerData, std::string bodyData, Client *client) {
    
//     Response *response = client->getResponse();
//     Request *request = new Request();
    
//     std::string line;

//     std::istringstream headerStream(headerData);
//     std::istringstream bodyStream(bodyData);

//     std::string dataStr(headerData + bodyData);

//     //controlla che la richiesta sia terminata in modo corretto con \r\n\r\n
//     if (dataStr.find("\r\n\r\n") == std::string::npos) {
//         response->setStatusCode(400);
//         return NULL;
//     }

//     //controlla che la richiesta abbia un body oppure no
//     if(!bodyData.empty())
//         request->setHasBody(true);
   
//     if (std::getline(headerStream, line) && this->extractFirstLine(request, response, line) != SUCCESS){
//         response->setStatusCode(400);
//         return NULL;
//     }   
    
//     this->extractHeaders(request, headerStream);

//     if(this->extractBody(request, bodyStream) != SUCCESS){
//         response->setStatusCode(400);
//         return NULL;
//     }
    
//     return request;
// }

void Parser::validateResource(Client *client, Server *server)
{

    int fileType;
    std::string fileContent;

    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response)
        return;

    // TODO: atm is hardcoded to the root directory
    std::string filePath = server->getCwd() +  server->getRoot() + request->getUrl();
    
    //check if is dir or file
    //if dir
        // if index is provided inside location
            //check existance
            //serve index.html 
        // check dir listing
            //if is on
                // if url does not end with / add /
                //serve dir listing
            //if is off
                //serve 403
          


    //if file 
        //



















    //check if the requested resource is accessible
    fileType = this->checkResource(filePath, response);
    if(fileType == FAILURE){
        response->setBody(response->getErrorPage(response->getStatus()));
        return;
    }
    
    if(S_ISDIR(fileType)){
        if(*(filePath.rbegin()) != '/'){
            std::string newUrl = request->getUrl() + "/";
            request->setUrl(newUrl);
        }
        // TODO:  implement the directory listing
        std::string dirBody = fromDIRtoHTML(filePath, request->getUrl());
        
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