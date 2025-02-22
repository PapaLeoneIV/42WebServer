#ifndef UTILS_HPP
#define UTILS_HPP

#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <algorithm>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <set>


typedef int SOCKET;
typedef int ERROR;

#define MAX_REQUEST_SIZE 10*1024*1024 //2MB

#define TIMEOUT_SEC 5

#define SERVER_NAME "UrMom"

#define ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~:/?#[]@!$&'()*+,;=%"

#define VERSION "1.0.0"

//List of possible error in the program, used to return a string error message
//to check the actual error message, checkout functionn ErrToStr in Utlis.cpp
enum POSSIBLE_ERRORS{
    FAILURE = -1,
    SUCCESS,
    //BOOTING ERRORS
    ERR_RESOLVE_ADDR,
    ERR_SOCK_CREATION,
    ERR_SOCKET_NBLOCK,
    ERR_BIND,
    ERR_LISTEN,
    ERR_FCNTL,
    //MONITOR ERRORS
    ERR_SELECT,
    ERR_SEND,
    ERR_RECV,
    //PARSER ERRORS
    INVALID_METHOD,
    INVALID_URL,
    INVALID_VERSION,
    INVALID_MANDATORY_HEADER,
    INVALID_BODY,
    INVALID_BODY_LENGTH,
    INVALID_MAX_REQUEST_SIZE,
    INVALID_CONNECTION_CLOSE_BY_CLIENT,
    INVALID_REQUEST,
    MISSING_HEADER,
    INVALID_CONTENT_LENGTH,
    FILE_NOT_FOUND,
    FILE_READ_DENIED,
    INVALID_HEADER,
};


enum RequestStates {
    StateMethod,
    StatePostOrPut,
    
    StateSpaceAfterMethod,
    StateUrlBegin,
    StateUrlString,
    StateSpaceAfterUrl,

    StateVersion,
    StateVersion_H,
    StateVersion_HT,
    StateVersion_HTT,
    StateVersion_HTTP,
    StateVersion_HTTPSlash,
    StateVersion_HTTPSlashOne,
    StateVersion_HTTPSlashOneDot,
    StateVersion_HTTPSlashOneDotOne,
    StateFirstLine_CR,
    StateFirstLine_LF,
    
    StateHeaderKey,
    StateHeadersTrailingSpaceStart,
    StateHeaderValue,
    StateHeadersTrailingSpaceEnd,
    StateHeaders_CR,
    StateHeaders_LF,
    StateHeadersEnd_CR,
    
    StateEncodedSep,
    
    StateBodyStart,
    StateBodyPlainText,
    StateBodyBodyChunkedText,
    StateBodyChunkedNumber,
    StateBodyEnd_CR,
    StateBodyEnd_LF,
    StateChunkedEnd_LF,
    StateChunkedNumber_LF,
    StateChunkedNumber_CR,
    StateChunkedChunk_LF,
    StateChunkedChunk, 
    
    StateParsingComplete,
  
  };
  
  enum RequestMethods{
    GET,
    POST,
    DELETE,
  };
int handle_arguments(int argc , char **argv);

std::string to_lower(const std::string& input);

int strToHex(const std::string& str);

std::string fromDIRtoHTML(std::string dirPath, std::string url);

std::string readTextFile(std::string filePath);

std::string readFileBinary(std::string filePath);

std::string getMessageFromStatusCode(int status);

std::string getContentType(std::string& url, int status);

std::string removeHexChars(std::string& url);

std::string ErrToStr(int error);

std::string intToStr(int number);

int strToInt(std::string str);

std::string to_lowercase(const std::string& str);

std::string trim(const std::string& str);

ERROR checkPermissions(std::string fullPath,int mode);

std::string readBinaryStream(std::istringstream &stream, int size);

std::string extractBodyFromStream(std::istringstream &iss, const std::string &boundary);

std::vector<std::string> splitIntoSections(std::istringstream &iss);

std::map<std::string, std::string> extractSection(const std::string &section);

#endif