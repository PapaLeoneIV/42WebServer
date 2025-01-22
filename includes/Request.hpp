#ifndef REQUEST_HPP
#define REQUEST_HPP


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

typedef struct Content{
    std::string name;
    std::string filename;
    std::string type;
    std::string body;
    int         length;
} Content;

/**Request class, contains the data rappresentation that an incoming request might contain*/
class Request{

public:
    
    Request();
    Request(Request &other);
    ~Request();

    void                                print              (void);
    void                                printHeaders       (void);
    std::string                         &getMethod         (void);
    std::string                         &getUrl            (void);
    std::string                         &getVersion        (void);
    std::map<std::string, std::string>  &getHeaders        (void);
    std::string                         &getBody           (void);
    std::string                         &getContType       (void);
    int                                 &getContentLength  (void);
    std::string                         &getContentName    (void);
    std::string                         &getContentFilename(void);
    std::string                         &getBoundary       (void);

    bool                                &hasBody           (void);

    void                                setMethod           (std::string& method);
    void                                setUrl              (std::string& url);
    void                                setVersion          (std::string& version);
    void                                setHeaders          (std::map<std::string, std::string>& headers);
    void                                setBody             (std::string& body);
    void                                setHasBody          (bool hasBody);
    void                                setContType         (std::string& contType);
    void                                setContentLength    (int contentLength);
    void                                setContentName      (std::string& contentName);
    void                                setContentFilename  (std::string& contentFilename);
    void                                setBoundary         (std::string& boundary);

private:

    std::string     _method;  
    std::string     _url;     
    std::string     _version; 
    std::string     _body;   
    std::map<std::string, std::string>  _headers;
    

    std::string     _contType;
    int             _contentLength;
    std::string     _contentName;
    std::string     _contentFilename;
    std::string     _boundary;

    struct Content  _formFields;

    bool    _hasBody;

};

#endif 
