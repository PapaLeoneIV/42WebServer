#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

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
    void                                reset                 (void);
    void                                flush                 (void);
    std::string                         getErrorPage          (int status);

    private:

    std::string                         _finalResponse;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;
    std::string                         _content_type; 
    std::string                         _status_message; 
    int                                 _status;
};


#endif
