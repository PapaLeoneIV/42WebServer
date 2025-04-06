#include "../includes/Logger.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Request.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#define URL_MATCHING_ERROR_ALLOWANCE 3

unsigned int Parser::levenshteinDistance(const std::string& s1, const std::string& s2)
{
	const std::size_t len1 = s1.size(), len2 = s2.size();
	std::vector<std::vector<unsigned int> > d(len1 + 1, std::vector<unsigned int>(len2 + 1));

	d[0][0] = 0;
	for(unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
	for(unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

	for(unsigned int i = 1; i <= len1; ++i)
		for(unsigned int j = 1; j <= len2; ++j)
                      d[i][j] = std::min(std::min(  d[i - 1][j] + 1,
                                                    d[i][j - 1] + 1),
                                                    d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));  
	return d[len1][len2];
}

std::string Parser::findBestApproximationString(std::string url, std::vector<std::string> dictionary) {

    int min_distance = INT_MAX;
    std::string best_match = "";
    size_t i;

    for (i = 0; i < dictionary.size(); i++) {
        int distance = levenshteinDistance(url, dictionary[i]);
        if (distance < URL_MATCHING_ERROR_ALLOWANCE && distance < min_distance) {
            min_distance = distance;
            best_match = dictionary[i];
        }
    }

    return best_match;
}

std::string  Parser::getMatchingLocation(std::string url, Server *server){
    std::vector<std::string> dictionary;


    std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = server->getLocationDir().begin();
    for(; it != server->getLocationDir().end(); it++){
        dictionary.push_back(it->first);
    }
    if(url.find_last_of("?") != std::string::npos)
        url = url.substr(0, url.find_last_of("?"));
    if(url.find_last_of("/") != std::string::npos)
        url = url.substr(0, url.find_last_of("/"));

    return findBestApproximationString(url, dictionary);

}


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


    //Al momento viene soltanto scelto il configuration block ma non viene eseguito nessun altro check
    std::string bestMatch = this->getMatchingLocation(client->getRequest()->getUrl(), client->getServer());
    if(bestMatch.empty()){
        response->setStatusCode(404);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->state = StateParsingError;
        return;
    }

    //da qui in poi (credo) che vadano fatti dei check sulla risorsa richiesta basandoci sul location block
    std::map<std::string, std::vector<std::string> > locationConfig = server->getLocationDir()[bestMatch];
    
    
    
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
