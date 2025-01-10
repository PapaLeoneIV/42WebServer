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
    std::string extension = url.substr(url.find_last_of(".") + 1);
    char * urlC = new char[url.length() + 1];
    if(!extension.empty())
    {
        if (strcmp(urlC,  "/") == 0)    return "text/html";
        if (strcmp(urlC, ".css") == 0)  return "text/css";
        if (strcmp(urlC, ".csv") == 0)  return "text/csv";
        if (strcmp(urlC, ".gif") == 0)  return "image/gif";
        if (strcmp(urlC, ".htm") == 0)  return "text/html";
        if (strcmp(urlC, ".html") == 0) return "text/html";
        if (strcmp(urlC, ".ico") == 0)  return "image/x-icon";
        if (strcmp(urlC, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(urlC, ".jpg") == 0)  return "image/jpeg";
        if (strcmp(urlC, ".js") == 0)   return "application/javascript";
        if (strcmp(urlC, ".json") == 0) return "application/json";
        if (strcmp(urlC, ".png") == 0)  return "image/png";
        if (strcmp(urlC, ".pdf") == 0)  return "application/pdf";
        if (strcmp(urlC, ".svg") == 0)  return "image/svg+xml";
        if (strcmp(urlC, ".txt") == 0)  return "text/plain";
    }   
    return NULL; 
}

std::string analyzeUrl(std::string& url) {
    std::string result;
    for (std::size_t i = 0; i < url.length(); ++i) {
        if (url[i] == '%' && i + 2 < url.length()) {
            std::string hex = url.substr(i + 1, 2); // Extract the two hex characters after '%'
            char asciiChar = hexToAsciiChar(hex);
            result += asciiChar; // Append the ASCII character to the result
            i += 2; // Skip the processed hex characters
        } else {
            result += url[i]; // Keep other characters unchanged
        }
    }
    return result;
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