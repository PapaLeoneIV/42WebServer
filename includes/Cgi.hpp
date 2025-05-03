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
    void setArgs(std::string target, Client *client);
    char **getArgs() ;
    char **generateEnv(Client *client, std::string scriptPath);
    void execute(Client *client);
    void freeEnv(char **envp);
    void setEnv(char **envp);
    char **getEnv() const;
    void setCGIState(int state);
    int getCGIState();
    int *getPipeIn();
    int *getPipeOut();
    int getCgiPid();
private:
    int cgi_state;
    char **args;
    char **envp;
    int pipe_in[2];
    int pipe_out[2];
    int cgi_pid;
    pid_t pid;
};



#endif // CGI_HPP