#ifndef RESPONSE_HPP
#define RESPONSE_HPP


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
class Response{
    
    public:

    Response();
    Response(int status, const char *status_message);
    Response(int status, std::string status_message);
    ~Response();

    void                                print                 (void);
    void                                prepareResponse       (void);
    std::string                         getErrorPage          (int status);

    void                                fillStatusLine        (void);
    void                                fillHeader            (std::string headerKey, std::string headerValue);
    
    std::string                         &getResponse          (void);
    std::string                         &getBody              (void);
    std::string                         &getContentType       (void);
    std::string                         &getStatusMessage     (void);
    int                                 &getStatus            (void);
    std::map<std::string, std::string>  &getHeaders           (void);

    void                                setResponse           (char *response);
    void                                setBody               (std::string body);
    void                                setContentType        (std::string content_type);
    void                                setStatusMessage      (std::string status_message);
    void                                setStatusCode         (int status);
    void                                setHeaders            (std::string key, std::string value);


    private:

    std::string                         _finalResponse;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;
    std::string                         _content_type; 
    std::string                         _status_message; 
    int                                 _status;
};


#endif