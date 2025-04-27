#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>
#include <unistd.h>

class Client;

class Cgi {


public:
    Cgi();
    ~Cgi();
    void reset();
    void setArgs(const std::vector<std::string> &args);
    void setEnv(Client *client, std::string scriptPath);
    void execute();

private:
    int pipe_in[2];
    int pipe_out[2];
    pid_t pid;
};



#endif // CGI_HPP