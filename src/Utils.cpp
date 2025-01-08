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