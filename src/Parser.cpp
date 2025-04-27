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
    if(url.find_last_of("/") != std::string::npos )
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
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response)
        return;
    
    if(server->getServerDir().find("client_max_body_size") != server->getServerDir().end() 
        && (size_t)strToInt(server->getServerDir()["client_max_body_size"][0]) < request->getBody().size()) {
            response->setStatusCode(404);
            response->setBody(getErrorPage(response->getStatus(), client->getServer()));
            request->setState(StateParsingError);
            return;    
    }

    std::string bestMatchingLocation = this->getMatchingLocation(client->getRequest()->getUrl(), client->getServer());
    if(bestMatchingLocation.empty()){
        response->setStatusCode(404);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->setState(StateParsingError);
        return;
    }
    std::map<std::string, std::vector<std::string> > locationConfig = server->getLocationDir()[bestMatchingLocation];
    
    //controllare che l host sia lo stesso nel file di config
    if(request->getHeaders().find("host") == request->getHeaders().end()){
        response->setStatusCode(400);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->setState(StateParsingError);
        return;
    }
    
    std::string reqHost =  request->getHeaders()["host"].substr(0, request->getHeaders()["host"].find_last_of(":"));

    //normalize url, i m doing the same for the configuration file in int ConfigParser::parseHostValues(directiveValueVector v)
    if(reqHost == "localhost" && server->getServerDir()["host"][0] == "127.0.0.1"){ reqHost = "127.0.0.1";}
    else if(reqHost == "127.0.0.1" && server->getServerDir()["host"][0] == "localhost"){ reqHost = "localhost";}
    
    if(reqHost != server->getServerDir()["host"][0]){
        response->setStatusCode(400);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->setState(StateParsingError);
        return;
    }
    
    std::string reqPort = request->getHeaders()["host"].substr(request->getHeaders()["host"].find_last_of(":") + 1, request->getHeaders()["host"].size());
    if(reqPort != server->getServerDir()["listen"][0]){
        response->setStatusCode(400);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->setState(StateParsingError);
        return;
    }

    //check for redirection
    if(locationConfig.find("return") != locationConfig.end()){
        response->setStatusCode(strToInt(locationConfig["return"][0]));
        if(locationConfig["return"].size() > 1) //if a path is present
            response->setHeaders("Location", locationConfig["return"][1]);
        return;
    }

    bool foundMethod = locationConfig["allow_methods"].size() ? false  : true;
    //controllare che il METHOD richiesto sia permesso nel location config
    for(size_t i = 0; i < locationConfig["allow_methods"].size(); i++){
        if(request->getMethod() == locationConfig["allow_methods"][i]){
            foundMethod = true;
        }
    }
    if(!foundMethod){
        response->setStatusCode(405);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->setState(StateParsingError);
        return;
    }


    //choose between location root or server root
    bool isRootInLocationDirective = locationConfig["root"].size(); 
    std::string rootPath = isRootInLocationDirective  ? locationConfig["root"][0] : server->getServerDir()["root"][0];


    //extract path and file requested
    std::string urlPath = request->getUrl().substr(0, request->getUrl().find_last_of("/"));
    std::string urlFile = request->getUrl().substr(request->getUrl().find_last_of("/") + 1, request->getUrl().size());

    //alias substitution
    if(locationConfig.find("alias") != locationConfig.end()){
        urlPath = locationConfig["alias"][0]; 
    }

    //if 'root' is present i need to attach root to location path
    std::string filePath =  rootPath + urlPath + urlFile;
    Logger::info("File path: " + filePath + " [" + intToStr(client->getSocketFd()) + "]");

    //check if the user is requesting a directory 
    std::string indexFile = locationConfig.find("index") != locationConfig.end() ? locationConfig["index"][0] : server->getServerDir()["index"][0];
    bool isAutoIndexOn = locationConfig.find("autoindex") != locationConfig.end() ? locationConfig["autoindex"][0] == "on" : false; 
    //attach index file to the path if the user is requesting a directory
    //and autoindex is off
    if (filePath[filePath.size() - 1] == '/' && !isAutoIndexOn) {
        filePath += indexFile;
    }










    if (request->getMethod() == "DELETE") {
        Logger::info("DELETE request for: " + filePath + " [" + intToStr(client->getSocketFd()) + "]");
        
        bool useDetailedResponse = isQueryParamValid(request->getUrl(), "details", false);
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

    



    int fileType;
    std::string fileContent;

    //checcko se la risorsa richiesta è accessibile
    fileType = this->checkResource(filePath, response);
    if(fileType == FAILURE){
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        return;
    }
    
    if(S_ISDIR(fileType) && isAutoIndexOn){
        if(filePath[filePath.size() - 1] != '/'){
            std::string newUrl = request->getUrl() + "/";
            request->setUrl(newUrl);
            filePath += "/";
        }
        std::string dirBody = fromDIRtoHTML(filePath, request->getUrl());
        
        if(dirBody.empty()){
            response->setStatusCode(500);
            return;
        }
        response->setBody(dirBody);
        return;
    } else {
        //read the content of the requested resource
        Logger::info("Reading file: " + filePath + " [" + intToStr(client->getSocketFd()) + "]");
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
}

Parser::~Parser(){}
