#include "../includes/Cgi.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Client.hpp"
#include <cstdlib>
#include <stdlib.h>

    void Cgi::setArgs(const std::vector<std::string> &args){
        (void)args;
    }
     //https://datatracker.ietf.org/doc/html/rfc3875#section-4.1
    void Cgi::setEnv(Client *client, std::string exec_path){

        Request *request = client->getRequest();
        Response *response = client->getResponse();
        if(!request || !response)
            return;
       
        setenv("AUTH_TYPE", "Basic", 1);
        

        if(request->getMethod() == "POST"){
            request->getHeaders().find("content-length") != request->getHeaders().end() 
            ? setenv("CONTENT_LENGTH", request->getHeaders()["content-length"].c_str(), 1) 
            : setenv("CONTENT_LENGTH", "", 1);
        
        request->getHeaders().find("content-type") != request->getHeaders().end() 
            ? setenv("CONTENT_TYPE", request->getHeaders()["content-type"].c_str(), 1) 
            : setenv("CONTENT_TYPE", "", 1);
        }
        
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);

        //dal subject dice 'Because you won’t call the CGI directly, use the full path as PATH_INFO.'
        setenv("PATH_INFO", exec_path.c_str(), 1);
        setenv("PATH_TRANSLATED", exec_path.c_str(), 1);
        setenv("REQUEST_URI",exec_path.c_str(), 1);
        setenv("SERVER_SOFTWARE", "URMOM", 1);
        //TODO: check if the request state machine is parsing it correctly
	    setenv("QUERY_STRING", request->getQueryParam().c_str(), 1);


        setenv("REQUEST_METHOD", request->getMethod().c_str(), 1);
        //TODO check if we need script-filename or only script name
	    setenv("SCRIPT_FILENAME", exec_path.c_str(), 1);
	    setenv("SCRIPT_NAME", exec_path.c_str(), 1);
	    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    }
    void Cgi::reset(){}
    void Cgi::execute(){}

    Cgi::Cgi(){};
    Cgi::~Cgi(){}
    // this->_env["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
	// this->_env["SCRIPT_NAME"] = cgi_exec;//
    // this->_env["SCRIPT_FILENAME"] = this->_cgi_path;
    // this->_env["PATH_INFO"] = this->_cgi_path;//
    // this->_env["PATH_TRANSLATED"] = this->_cgi_path;//
    // this->_env["REQUEST_URI"] = this->_cgi_path;//
    // this->_env["SERVER_NAME"] = req.getHeader("host");
    // this->_env["SERVER_PORT"] ="8002";
    // this->_env["REQUEST_METHOD"] = req.getMethodStr();
    // this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    // this->_env["REDIRECT_STATUS"] = "200";
	// this->_env["SERVER_SOFTWARE"] = "URMOM";
