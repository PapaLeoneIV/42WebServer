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
    
    // Ottengo info sul file prima di eliminarlo (per 200 OK)
    struct stat fileInfo;
    std::string fileDetails = "";
    if (useDetailedResponse && stat(filePath.c_str(), &fileInfo) == 0) {
        fileDetails = "File: " + filePath + "\n";
        fileDetails += "Size: " + intToStr(fileInfo.st_size) + " bytes\n";
        fileDetails += "Last modified: " + std::string(ctime(&fileInfo.st_mtime));
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

    if (remove(filePath.c_str()) != 0) {
        Logger::error("Parser", "Failed to delete file: " + filePath + " - " + std::string(strerror(errno)));
        response->setStatusCode(500);
        return FAILURE;
    }

    // Secondo HTTP 1.1:
    // - 204 No Content: quando non c'è bisogno di inviare un corpo nella risposta
    // - 200 OK: quando si vuole inviare un corpo nella risposta (es. conferma, statistiche, ecc.)
    if (useDetailedResponse) {
        response->setStatusCode(200);
        std::string successBody = "<html><body>\n<h1>File deleted successfully</h1>\n";
        successBody += "<p>Details:</p>\n";
        successBody += "<pre>\n" + fileDetails + "</pre>\n";
        successBody += "</body></html>\n";
        response->setBody(successBody);
    } else {
        response->setStatusCode(204);
    }
    return SUCCESS;
}

void Parser::setCgiEnv(Request *request, const std::string &scriptPath, const std::string &requestBody)
{
    std::string url = request->getUrl();
	std::string queryString = getQueryString(url);

	// variabili d'ambiente minime richieste
	setenv("REQUEST_METHOD", request->getMethod().c_str(), 1);
	setenv("CONTENT_LENGTH", std::to_string(requestBody.length()).c_str(), 1);
	setenv("CONTENT_TYPE", request->getHeader("Content-Type").c_str(), 1);
	setenv("QUERY_STRING", queryString.c_str(), 1);
	setenv("SCRIPT_FILENAME", scriptPath.c_str(), 1);
	setenv("SCRIPT_NAME", url.c_str(), 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	//forse da aggiungere altre variabili d'ambiente
}

void Parser::executeCgiScript(int inputPipe[2], int outputPipe[2], Request *request, const std::string &scriptPath, const std::string &requestBody)
{
	
	close(STDIN_FILENO);
	dup2(inputPipe[0], STDIN_FILENO);
	close(STDOUT_FILENO);
	dup2(outputPipe[1], STDOUT_FILENO);
	close(inputPipe[0]);
	close(inputPipe[1]);
	close(outputPipe[0]);
	close(outputPipe[1]);

	setCgiEnv(request, scriptPath, requestBody);

	execl(scriptPath.c_str(), scriptPath.c_str(), NULL); //execl invece di execve perche' usa la variabile d'ambiente con setenv

	Logger::error("Parser", "Failed to execute CGI script: " + std::string(strerror(errno)));
	exit(EXIT_FAILURE);
}

void Parser::executeCgi(Client *client, Server *server, const std::string &requestBody)
{
	Request *request = client->getRequest();
    Response *response = client->getResponse();

    std::string url = request->getUrl();
    std::string scriptPath = server->getRoot() + url;

	//check se eseguibile oppure se il file esiste ma NON ha il permesso di esecuzione per l'utente proprietario
	struct stat st;
    if (stat(scriptPath.c_str(), &st) == -1 || !(st.st_mode & S_IXUSR)) {
        response->setStatusCode(404);
        response->setBody(getErrorPage(404, server));
        Logger::error("Parser", "CGI script not found or not executable: " + scriptPath);
        return;
    }
	
	// pipe processo CGI
    int inputPipe[2];
    int outputPipe[2];
    
    if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0) {
        response->setStatusCode(500);
        response->setBody(getErrorPage(500, server));
        Logger::error("Parser", "Failed to create pipes for CGI");
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        close(inputPipe[0]);
        close(inputPipe[1]);
        close(outputPipe[0]);
        close(outputPipe[1]);
        
        response->setStatusCode(500);
        response->setBody(getErrorPage(500, server));
        Logger::error("Parser", "Failed to fork for CGI execution");
        return;
    }
    
    if (pid == 0) { 
		executeCgiScript(inputPipe, outputPipe, request, scriptPath, requestBody);
	}
	//da implementare la gestione del processo padre

}

void Parser::handlePostRequest(Client *client, Server *server, const std::string &uploadDir)
{
	Logger::info("Credevi la POST fosse gia' implementata.... cor cazzo");
	
	Request *request = client->getRequest();
    Response *response = client->getResponse();
    
    // Ottieni Content-Type e Content-Length
    std::string contentType = request->getHeader("Content-Type");
    std::string contentLength = request->getHeader("Content-Length");
    
    // Verifica che Content-Length sia presente
    if (contentLength.empty()) {
        response->setStatusCode(411); // Length Required
        response->setBody(getErrorPage(411, server));
        Logger::error("Parser", "Missing Content-Length header in POST request");
        return;
	}
	std::string body = request->getBody();
   	executeCgi(client, server, body);
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
                Logger::info("Resource deleted successfully (204 No Content): " + filePath + " [" + intToStr(client->getSocketFd()) + "]");
            } else {
                Logger::info("Resource deleted successfully (200 OK): " + filePath + " [" + intToStr(client->getSocketFd()) + "]");
            }
        } else {
            Logger::error("Parser", "Failed to delete resource: " + filePath + " (Status: " + intToStr(response->getStatus()) + ") [" + intToStr(client->getSocketFd()) + "]");

            response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        }
        return;
    }

	if (request->getMethod() == "POST") {
		Logger::info("POST request for: " + filePath + " [" + intToStr(client->getSocketFd()) + "]");

		std::string uploadDir = server->getRoot() + "/uploads"; //dir dedicata 

		//check se dir esiste, nel caso la creo
		struct stat st;
		if(stat(uploadDir.c_str(), &st) != 0) {
			if(mkdir(uploadDir.c_str(), 0755) != 0) {
				response->setStatusCode(500);
				response->setBody((getErrorPage(500, client->getServer())));
				Logger::error("Parser", "Failed to create upload directory: " + uploadDir);
				return;
			}
		}

		handlePostRequest(client, server, uploadDir);
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
