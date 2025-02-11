#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"


// ERROR Parser::extractFirstLine(Request *request, Response *response, std::string firstLine) {
    
//     std::istringstream firstLineStream(firstLine);
    
//     std::string method, url, version;
    
//     firstLineStream >> method >> url >> version;
    
//     //TODO allowd methods deve essere construito basandosi sulle informazioni presenti nel config file del server
//     switch (this->_allowd_methods.count(method)) {
//         case true:
//             if (this->_implemnted_methods.find(method) == this->_implemnted_methods.end()) {
//                 // not permitted
//                 response->setStatusCode(405);
//                 response->setHeaders("Allow", "GET, POST, DELETE");
//                 return INVALID_HEADER;
//             }
//             break;
//         case false:
//                 // not implemented
//                 response->setStatusCode(501);
//                 response->setHeaders("Allow", "GET, POST, DELETE");
//                 return INVALID_HEADER;
//     }

//     request->setMethod(method);


//     //se l url contiene "%" allora i caratteri che seguono sono codificati in esadecimale e vanno tradotti
//     url = removeHexChars(url);
    
//     if (url.find_first_not_of(ALLOWED_CHARS) != std::string::npos || url.find_first_of("/") != 0) {
//         response->setStatusCode(400);
//         return INVALID_HEADER;
//     }
    
//     request->setUrl(url);

//     //Unica versione supportata da subject è HTTP/1.1
//     if (this->_allowd_versions.find(version) == this->_allowd_versions.end()) {
//         response->setStatusCode(505);
//         return INVALID_HEADER;
//     }
    
//     request->setVersion(version);
//     return SUCCESS;
// }


/**
 * Gli headers vengono estratti ed inseriti in una mappa, ci basiamo semplicemente 
 * sulla presenza del ":" per dividere il nome del header dal valore.
 * 
 */
// void Parser::extractHeaders(Request *request, std::istringstream &headerStream) {
    
//     std::string line;
//     std::map<std::string, std::string> headers;
//     while (std::getline(headerStream, line) && line != "\r") {
    
//         line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

//         std::string::size_type colonPos = line.find(':');
//         if (colonPos != std::string::npos) {

//             std::string headerName = line.substr(0, colonPos);
//             std::string headerValue = line.substr(colonPos + 1);

//             headerValue.erase(0, headerValue.find_first_not_of(" \t"));

//             std::string::size_type endPos = headerValue.find_last_not_of(" \t");
//             if (endPos != std::string::npos) {
//                 headerValue.erase(endPos + 1);
//             }

//             //TODO nell RFC c'è scritto che gli headers sono case-insensitive, non so se mi piace perche in altre parte del
//             //codice ho degli string literals dove non sono in lower case
//             for (std::string::iterator it = headerName.begin(); it != headerName.end(); ++it) {
//                 *it = std::tolower(*it);
//             }

//             headers[headerName] = headerValue;
//         }
//     }
//     // TODO: check if the headers are valid
//     Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/20
//     request->setHeaders(headers);
// }


/*  After we got the headers from the request, 
*   we should check what type of request is GET, POST, DELETE 
*   and if it has a body extract it accordingly reference blog(https://http.dev/post) 
*/
// ERROR Parser::extractBody(Request *request, std::istringstream &bodyStream) {

//     if(request->getMethod() == "GET" || request->getMethod() == "DELETE") {
//         //We can ignore the body if is a GET or DELETE request, le info sono già presenti nell url
//         std::string empty = "";
//         request->setBody(empty); 

//     } else if (request->getMethod() == "POST" && request->hasBody()) {

//         if(request->getContType() == "text/plain"){
         
//             std::string bodyContent((std::istreambuf_iterator<char>(bodyStream)), std::istreambuf_iterator<char>());
//             //La presenza di Hexadecimal chars è segnata dalla presenza di "%" 
//             bodyContent = removeHexChars(bodyContent);
//             request->setBody(bodyContent);
        
//         } else if (request->getContType() == "multipart/form-data"){
        
//             std::string contTypeVal = request->getHeaders()["content-type"];
//             std::string boundary = contTypeVal.substr(contTypeVal.find("boundary=") + 9);
//             request->setBoundary(boundary);

//             /***
//              * Body:
//              * [name(nome)][Riccardo]
//              * [name(cognome)][Leone]
//              * [name(image.jpg)][jpg]
//              */

//             this->parseMultipart(request, bodyStream, request->getBoundary());
        
//         } else if (request->getContType() == "application/x-www-form-urlencoded"){
        
//             std::string bodyContent((std::istreambuf_iterator<char>(bodyStream)), std::istreambuf_iterator<char>());
//             bodyContent = removeHexChars(bodyContent);

//             request->setBody(bodyContent);
        
//         } else if(request->getContType() == "application/json"){
//             std::cout << "Error: JSON not supported yet!" << std::endl;
//             return INVALID_REQUEST;
//         } else {
//             std::cout << "Error: Content type not supported!" << std::endl;
//             return INVALID_REQUEST;
//         }        
//     }
//     return SUCCESS;
// }

// void Parser::parseMultipart(Request *request, std::istringstream &formDataStream, std::string boundary) {

//     std::string sections = extractBodyFromStream(formDataStream, boundary);


//     std::istringstream sectionsStream(sections);
    
//     std::vector<std::string> sezioni = splitIntoSections(sectionsStream);
//     int i = 0;
//     for (std::vector<std::string>::const_iterator it = sezioni.begin(); it != sezioni.end(); ++it) {

//         std::map<std::string, std::string> extractedData = extractSection(*it);
//         //TODO: handle multiple form field into Request Object
//         request->setContentName(extractedData["name"]);
//         request->setContentFilename(extractedData["filename"]);
//         request->setContType(extractedData["contentType"]);
//         request->setBody(extractedData["body"]);
        
        
//         std::vector<char> bodyDataVector(extractedData["body"].begin(), extractedData["body"].end());
//         std::ofstream outFile("output.jpg", std::ios::out | std::ios::binary);
//         if (outFile) {
//             outFile.write(bodyDataVector.data(), bodyDataVector.size());
//             outFile.close();
//         }
//         i++;
//     }             
// }


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