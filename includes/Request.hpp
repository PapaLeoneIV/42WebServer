#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

typedef int SOCKET;
typedef int ERROR;

/**Request class, contains the data rappresentation that an incoming request might contain*/
class Request{

public:
    
    Request();
    Request(Request &other);
    ~Request();


    int  consume(std::string buffer);
    
    void print_Request();

    void  printHeaders  (void);
    std::string &getMethod  (void);
    std::string &getUrl (void);
    std::string &getVersion (void);
    std::map<std::string, std::string>  &getHeaders (void);

    bool  &hasBody  (void);
    void  setMethod (std::string& method);
    void  setUrl  (std::string& url);
    void  setVersion  (std::string& version);
    void  setHeaders  (std::map<std::string, std::string>& headers);
    void  setBody (std::string& body);
    void  setHasBody  (bool hasBody);

    void reset(void);

    std::map<std::string, std::string> headers;
    std::string raw;
    int state;
    std::string method;
    std::string url;
    std::string version;
    std::string body;
    std::string content;
    bool has_body;
    bool is_chunked;
    int error;

    size_t number;
    int body_counter;
    size_t encoded_counter;
    std::string encoded_char;
    std::string headers_key;
    std::map<int, std::string> methods;

private:

};

#endif 
