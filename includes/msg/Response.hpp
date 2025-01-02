#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <iostream>
#include <vector>


class Response{
    
    public:

    Response();
    Response(int status, const char *status_message);
    ~Response();


    void                                prepareResponse       (std::string b);

    std::string                         &getResponse          ();
    int                                 &getContentLength     ();
    int                                 &getFlags             ();
    std::string                         &getBody              ();
    std::string                         &getContentType       ();
    std::string                         &getStatusMessage     ();
    int                                 &getStatus            ();
    std::map<std::string, std::string>  &getHeaders           ();

    void                                setResponse           (char *response);
    void                                setContentLength      (int size);
    void                                setFlags              (int flags);

    private:

    std::string                         _response;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;
    std::string                         _content_type; 
    int                                 _content_length;
    std::string                         _status_message; 
    int                                 _status;
    int                                 _flags;
};


#endif