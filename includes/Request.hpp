#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <unistd.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <string>
#include <map>
enum states{
  Method_state,
  Post_or_Put_state,
  Space_after_method,
  Url_begin_state,
  Url_string_state,
  Space_after_url,
  Http_version_state

};

enum methods{
  GET,
  POST,
  DELETE,
  PUT,
  HEAD,
};

typedef int SOCKET;
typedef int ERROR;

/**Request class, contains the data rappresentation that an incoming request might contain*/
class Request{

public:
    
    Request();
    Request(Request &other);
    ~Request();


    void                                feed(std::string buffer);
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
  //parsing
  int state;
  int error;
  std::string     _method;  

private:

    std::string     _url;     
    std::string     _version; 
    std::string     _body;   
    std::map<std::string, std::string>  _headers;


  
    

    std::string     _contType;
    int             _contentLength;
    std::string     _contentName;
    std::string     _contentFilename;
    std::string     _boundary;

    bool    _hasBody;

};

#endif 
