#include "Utils.hpp"



std::string ErrToStr(int error) {
    switch (error) {
        case SUCCESS:
            return "Success";
        //BOOTING ERRORS
        case ERR_RESOLVE_ADDR:
            return "Could not resolve address";
        case ERR_SOCK_CREATION:
            return "Error: socket creation failed";
        case ERR_SOCKET_NBLOCK:
            return "Error: setting socket to non-blocking failed";
        case ERR_BIND:
            return "Error: bind failed";
        case ERR_LISTEN:
            return "Error: listen failed";
        //MONITOR ERRORS
        case ERR_SELECT:
            return "Error: select failed";
        case ERR_SEND:
            return "Error: send failed";
        case ERR_RECV:
            return "Error: recv failed: closing connection";
        //PARSER ERRORS
        case INVALID_METHOD:
            return "Error: the method is not supported (yet)";
        case INVALID_URL:
            return "Error: Invalid URL";
        case INVALID_VERSION:
            return "Error: HTTP version not supported";
        case INVALID_MANDATORY_HEADER:
            return "Error: Missing mandatory header";
        case INVALID_BODY:
            return "Error: Invalid body";
        case INVALID_BODY_LENGTH:
            return "Error: Invalid body length";
        case INVALID_MAX_REQUEST_SIZE:
            return "Error: Request too long";
        case INVALID_CONNECTION_CLOSE_BY_CLIENT:
            return "Error: Connection closed by client";
        case INVALID_REQUEST:
            return "Error: Invalid request";
        case INVALID_CONTENT_LENGTH:
            return "Error: Invalid content length";
        case ERR_FCNTL:
            return "Error: fcntl failed";
        case FILE_NOT_FOUND:
            return "Error: File not found";
        case FILE_READ_DENIED:
            return "Error: Read access denied";
        default:
            return "Unknown Error";
    }
}
static char hexToAsciiChar(const std::string& hex) {
    if (hex.length() != 2) {
        throw std::invalid_argument("Hex string must be exactly 2 characters long.");
    }

    int decimalValue;
    std::istringstream(hex) >> std::hex >> decimalValue;
    return static_cast<char>(decimalValue);
}

std::string getContentType(std::string& url) {
    if (url ==  "/") return "text/html"; 
    size_t idx = url.find_last_of(".");
    if(idx == std::string::npos) return "text/plain";

    std::string extension = url.substr(idx);
    if(extension.empty()) return "text/plain";
    std::string urlC = &extension[0];
    if (urlC == ".css")  {return "text/css";}
    if (urlC == ".csv")  {return "text/csv";}
    if (urlC == ".gif")  {return "image/gif";}
    if (urlC == ".htm")  {return "text/html";}
    if (urlC == ".html") {return "text/html";}
    if (urlC == ".ico")  {return "image/x-icon";}
    if (urlC == ".jpeg") {return "image/jpeg";} 
    if (urlC == ".jpg")  {return "image/jpeg";}
    if (urlC == ".js")   {return "application/javascript";}
    if (urlC == ".json") {return "application/json";}
    if (urlC == ".png")  {return "image/png";}
    if (urlC == ".pdf")  {return "application/pdf";}
    if (urlC == ".svg")  {return "image/svg+xml";}
    if (urlC == ".txt")  {return "text/plain";}

    return "text/plain";
}


//TODO add checks
std::string analyzeUrl(std::string& url) {
    std::string result;
    for(std::size_t i = 0; i < url.length(); ++i) {
        if (url[i] == '%' && i + 2 < url.length()) {
            std::string hex = url.substr(i + 1, 2); 
            result += hexToAsciiChar(hex); 
            i += 2;
        } else {
            result += url[i];
        }
    }
    size_t pos;
    while ((pos = result.find("..")) != std::string::npos) {
        result.erase(pos, 2);
    }
    while ((pos = result.find("../")) != std::string::npos) {
        result.erase(pos, 3);
    }
    return result;
}

ERROR checkPermissions(std::string fullPath, int mode) {
    ERROR error = 0;
    if ((error = access(fullPath.c_str(), mode))) {
        return error;
    }
    return SUCCESS;
}


int strToInt(std::string str) {
    std::stringstream ss(str);
    int number;
    ss >> number;
    return number;
}

std::string intToStr(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}


std::string to_lowercase(const std::string& str) {
    std::string lower_str = str;
    for (size_t i = 0; i < lower_str.size(); ++i) {
        lower_str[i] = static_cast<char>(std::tolower(lower_str[i]));
    }
    return lower_str;
}