#ifndef UTILS_HPP
#define UTILS_HPP


#include <string>
#include <iostream>
#include <sstream>

typedef int SOCKET;
typedef int ERROR;

#define MAX_REQUEST_SIZE 4096

#define TIMEOUT_SEC 5

enum err_parsing{
    SUCCESS,
    INVALID_METHOD,
    INVALID_URL,
    INVALID_VERSION,
    INVALID_HEADER,
    INVALID_BODY,
    INVALID_BODY_LENGTH,
    INVALID_REQUEST_SIZE,
    INTERNAL_RECV_ERROR,
    INVALID_CONNECTION_CLOSE_BY_CLIENT,
};

std::string error_to_string             (int error);

std::string int_to_string               (int number);

std::string to_lowercase                (const std::string& str);

std::string trim                        (const std::string& str);


#endif