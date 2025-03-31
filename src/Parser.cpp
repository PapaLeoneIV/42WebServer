#include "../includes/Parser.hpp"
#include "../includes/Request.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"


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


int Parser::deleteResource(std::string filePath, Response *response, bool useDetailedResponse) {
    // Checkko se esiste
    if (access(filePath.c_str(), F_OK) != 0) {
        response->setStatusCode(404);
        return FAILURE;
    }
    
    // Ottengo info sul file (per 200 OK)
    struct stat fileInfo;
    std::string fileDetails = "";
    if (useDetailedResponse && stat(filePath.c_str(), &fileInfo) == 0) {
        fileDetails = "File: " + filePath + "\n";
        fileDetails += "Size: " + intToStr(fileInfo.st_size) + " bytes\n";
        fileDetails += "Last modified: " + std::string(ctime(&fileInfo.st_mtime));
        fileDetails += "Content: " + this->readFile(filePath, response);
    }
    
    // Checkko se è una directory
    if (stat(filePath.c_str(), &fileInfo) == 0 && S_ISDIR(fileInfo.st_mode)) {
        response->setStatusCode(403);
        Logger::error("Parser", "Cannot delete directory: " + filePath);
        return FAILURE;
    }

    // Checkko i permessi
    int fileType = this->checkResource(filePath, response, W_OK);
    if (fileType == FAILURE) {
        Logger::error("Parser", "Resource check failed: " + filePath + " (Status: " + intToStr(response->getStatus()) + ")");
        return FAILURE;
    }


    // Secondo HTTP 1.1:
    // - 204 No Content: quando non c'è bisogno di inviare un corpo nella risposta
    // - 200 OK: quando si vuole inviare un corpo nella risposta (es. conferma, statistiche, ecc.)
    if (useDetailedResponse) {
        response->setStatusCode(200);
        std::string successBody = "<html><body>\n<h1>DELETE request processed successfully</h1>\n";
        successBody += "<p>The server has accepted the DELETE request for the following resource:</p>\n";
        successBody += "<pre>\n" + fileDetails + "</pre>\n";
        successBody += "<p>Note: Resources are not actually deleted as per server configuration.</p>\n";
        successBody += "</body></html>\n";
        response->setBody(successBody);
    } else {
        response->setStatusCode(204);
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

    // TODO: atm is hardcoded to the root directory
    // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/6

    std::string url = request->getUrl();
    // rimuovo i parametri di query dall'URL per ottenere il percorso del file
    std::string cleanUrl = url;
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos) {
        cleanUrl = url.substr(0, queryPos);
    }
    std::string filePath = server->getCwd() + server->getRoot() + cleanUrl;
    
    if (request->getMethod() == "DELETE") {
        Logger::info("DELETE request for: " + filePath + " [" + intToStr(client->getSocketFd()) + "]");
        
        bool useDetailedResponse = isQueryParamValid(url, "details", false);
        int result = this->deleteResource(filePath, response, useDetailedResponse);
        if (result == SUCCESS) {
            if (response->getStatus() == 204) {
                Logger::info("DELETE request processed successfully (204 No Content): " + filePath + " [" + intToStr(client->getSocketFd()) + "]");
            } else {
                Logger::info("DELETE request processed successfully (200 OK): " + filePath + " [" + intToStr(client->getSocketFd()) + "]");
            }
        } else {
            Logger::error("Parser", "Failed to process DELETE request: " + filePath + " (Status: " + intToStr(response->getStatus()) + ") [" + intToStr(client->getSocketFd()) + "]");

            response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        }
        return;
    }

    //checcko se la risorsa richiesta è accessibile
    fileType = this->checkResource(filePath, response);
    if(fileType == FAILURE){
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        return;
    }
    
    if(S_ISDIR(fileType)){
        if(*(filePath.rbegin()) != '/'){
            std::string newUrl = cleanUrl + "/";
            request->setUrl(newUrl);
            filePath += "/";
        }
        // TODO:  implement the directory listing feature based on the config file
        // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/5
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
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        return;
    }

    response->setBody(fileContent);

    return ;
}


Parser::Parser() {

    this->_allowd_versions.insert("HTTP/1.1 ");
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
