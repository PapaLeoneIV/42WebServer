#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Utils.hpp"


typedef struct Content{
    std::string type;
    std::string name;
    std::string filename;
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

    std::map<std::string, std::string> formFields;

    bool    _hasBody;

};

#endif 
