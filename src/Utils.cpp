#include "Utils.hpp"



std::string error_to_string(int error) {
    switch (error) {
        case SUCCESS:
            return "Success";
        case INVALID_METHOD:
            return "Invalid Method";
        case INVALID_URL:
            return "Invalid URL";
        case INVALID_VERSION:
            return "Invalid Version";
        case INVALID_HEADER:
            return "Invalid Header";
        case INVALID_BODY:
            return "Invalid Body";
        case INVALID_BODY_LENGTH:
            return "Invalid Body Length";
        default:
            return "Unknown Error";
    }
}

std::string int_to_string(int number) {
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