#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Utils.hpp"


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

   
    void                                setMethod           (std::string& method);
    void                                setUrl              (std::string& url);
    void                                setVersion          (std::string& version);
    void                                setHeaders          (std::map<std::string, std::string>& headers);
    void                                setBody             (std::string& body);

private:

    std::string                         _method;  
    std::string                         _url;     
    std::string                         _version; 
    std::string                         _body;   
    std::map<std::string, std::string>  _headers;

};

#endif 
