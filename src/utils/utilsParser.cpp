#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"


void Parser::decomposeFirstLine(Request *request, std::string firstLine) {
    std::istringstream firstLineStream(firstLine);
    std::string method, url, version;
    firstLineStream >> method >> url >> version;
    
    //if we find % we should parse the next two char as HEX and replace it with the actual char
    url = removeHexChars(url);

    request->setMethod(method);
    request->setUrl(url);
    request->setVersion(version);
}

void Parser::decomposeHeaders(Request *request, std::string headerData) {
    std::istringstream headerStream(headerData);
    std::string line;
    std::map<std::string, std::string> headers;

    while (std::getline(headerStream, line) && line != "\r") {
        // Remove any trailing carriage return
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        std::string::size_type colonPos = line.find(':');
        if (colonPos != std::string::npos) {

            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            headerValue.erase(0, headerValue.find_first_not_of(" \t"));

            std::string::size_type endPos = headerValue.find_last_not_of(" \t");
            if (endPos != std::string::npos) {
                headerValue.erase(endPos + 1);
            }

            for (std::string::iterator it = headerName.begin(); it != headerName.end(); ++it) {
                *it = std::tolower(*it);
            }

            headers[headerName] = headerValue;
        }
    }
    request->setHeaders(headers);
}

void Parser::parseChunked(Request *request, std::istringstream &bodyStream) {
    std::string line;
    std::string joined;
    while (std::getline(bodyStream, line)) {
        if (line == "0\r") {
            break;
        }
        if(line.find("\r") != std::string::npos){
            line = line.substr(0, line.find("\r"));
        }
        int chunkSize = strToInt(line);
        std::string chunk = readBinaryStream(bodyStream, chunkSize);
        joined += chunk;
    }

    request->setBody(joined);
}

void Parser::decomposeBody(Request *request, std::string bodyData) {
    std::istringstream bodyStream(bodyData);
    //After we got the headers from the request, 
    //we should check what type of request is GET, POST, DELETE 
    //and if it has a body decompose it accordingly reference blog(https://http.dev/post)
    std::cout << "BODY DATA: " << bodyData << std::endl;
    if(request->getMethod() == "POST" && request->hasBody()) {

        if(request->getHeaders().find("content-length") != request->getHeaders().end()){
            std::string contentLength = request->getHeaders()["content-length"];
            int contLength = strToInt(contentLength);
            request->setContentLength(contLength);
        } else if(request->getHeaders().find("transfer-encoding") != request->getHeaders().end()){
            this->parseChunked(request, bodyStream);
            //TODO handle chunked encoding
        }
            std::string contTypeValue = request->getHeaders()["content-type"];
            //mi prendo il value che segue l header "content type"
            if (contTypeValue.find(";") != std::string::npos){
                contTypeValue = contTypeValue.substr(0, contTypeValue.find(";"));
            }
            if (contTypeValue == "text/plain"){
                
                std::string bodyContent((std::istreambuf_iterator<char>(bodyStream)), std::istreambuf_iterator<char>());;
                bodyContent = removeHexChars(bodyContent);
                request->setBody(bodyContent);
            } else if (contTypeValue == "multipart/form-data") {

                std::string tmp = request->getHeaders()["content-type"];
                std::string tmpBoundary = tmp.substr(tmp.find("boundary=") + 9);
                request->setBoundary(tmpBoundary);
                this->parseMultipart(request, bodyStream, request->getBoundary());

            } else if (contTypeValue == "application/x-www-form-urlencoded") {
                
                //TODO handle other content types
                std::string bodyContent((std::istreambuf_iterator<char>(bodyStream)), std::istreambuf_iterator<char>());
                bodyContent = removeHexChars(bodyContent);
                request->setBody(bodyContent);
            }

    }
}


// Parse the multipart form data
void Parser::parseMultipart(Request *request, std::istringstream &formDataStream, std::string boundary) {

    std::string sections = joinBoundaries(formDataStream, boundary);


    std::istringstream sectionsStream(sections);
    
    std::vector<std::string> sezioni = splitIntoSections(sectionsStream);

    // int totalLength = 0;
    // std::string totat;
    // for (std::vector<std::string>::const_iterator it = sezioni.begin(); it != sezioni.end(); ++it) {
    //     totalLength += it->length();
    //     totat += *it;
    // }

    int i = 0;
    for (std::vector<std::string>::const_iterator it = sezioni.begin(); it != sezioni.end(); ++it) {
        
       

        std::map<std::string, std::string> extractedData = extractSection(*it);

        request->setContentName(extractedData["name"]);
        request->setContentFilename(extractedData["filename"]);
        request->setContType(extractedData["contentType"]);
        request->setBody(extractedData["body"]);
        
        i++;
        // std::cout << "+++++++++" << request->getBody() << std::endl;   
        // std::cout << "+++++++++" << request->getContentFilename() << std::endl;   
        // std::cout << "+++++++++" << request->getContType() << std::endl;   
        // std::cout << "+++++++++" << request->getContentName() << std::endl;   
         std::cout << "+++++++++" << extractedData["body"] << std::endl;
        std::vector<char> bodyDataVector(extractedData["body"].begin(), extractedData["body"].end());
        std::ofstream outFile("output.jpg", std::ios::out | std::ios::binary);
        if (outFile) {
            outFile.write(bodyDataVector.data(), bodyDataVector.size());
            outFile.close();
        }
        }
       
        

       
                  
}




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