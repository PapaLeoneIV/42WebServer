#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <sys/socket.h>
#include <cstddef>
#include <map>
#include <vector>
#include <string>

typedef int SOCKET;
typedef int ERROR;


class Response{
    public:

    ERROR       send_response(SOCKET fd);
    void        prepare_response(std::string b);

    std::string get_response();
    int         get_content_length();
    int         get_flags();

    void        set_response(char *response);
    void        set_content_length(int size);
    void        set_flags(int flags);

                Response();
                Response(int status, const char *status_message);
                ~Response();


    private:

    std::string response;

    std::map<std::string, std::string> headers;
    
    std::string body;

    std::string content_type;
    
    std::string status_message;
    
    int     content_length;

    int     status;
    
    int     flags;
};


#endif