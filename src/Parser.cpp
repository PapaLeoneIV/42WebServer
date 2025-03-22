#include "../includes/Logger.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Request.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"

#define URL_MATCHING_ERROR_ALLOWANCE 3

unsigned int Parser::levenshtein_distance(const std::string& s1, const std::string& s2)
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
        int distance = levenshtein_distance(url, dictionary[i]);
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

void Parser::validateResource(Client *client, Server *server)
{
    int fileType;
    std::string fileContent;

    Request *request = client->getRequest();
    Response *response = client->getResponse();

    if(!request || !response)
        return;

    std::string bestMatch = this->getMatchingLocation(client->getRequest()->getUrl(), client->getServer());
    if(bestMatch.empty()){
        response->setStatusCode(404);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->state = StateParsingError;
        return;
    }
    std::map<std::string, std::vector<std::string> > locationConfig = server->getLocationDir()[bestMatch];
    // TODO: atm is hardcoded to the root directory
    // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/6
    std::string filePath = server->getCwd() +  server->getRoot() + request->getUrl();
    
    //check if the requested resource is accessible
    fileType = this->checkResource(filePath, response);
    if(fileType == FAILURE){
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        request->state = StateParsingError;
        return;
    }
    
    if(S_ISDIR(fileType)){
        if(*(filePath.rbegin()) != '/'){
            std::string newUrl = request->getUrl() + "/";
            request->setUrl(newUrl);
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