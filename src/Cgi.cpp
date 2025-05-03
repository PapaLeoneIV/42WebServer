#include "../includes/Cgi.hpp"
#include "../includes/Client.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include <cstdlib>
#include <cstring>
#include <stdlib.h>

void Cgi::setArgs(std::string target, Client *client) {
  Response *response = client->getResponse();
  
  char **args = (char **)calloc(3, sizeof(char *));
  if (!args) {
    Logger::error(__FILE__, "Allocating memory for args failed");
    response->setStatusCode(500);
    response->setBody(getErrorPage(response->getStatus(), client->getServer()));
    return;
  }
  args[0] = strdup(target.c_str());
  if (!args[0]) {
    free(args);
    Logger::error(__FILE__, "Allocating memory for args[0] failed");
    response->setStatusCode(500);
    response->setBody(getErrorPage(response->getStatus(), client->getServer()));
    return;

    return;
  }
  args[1] = strdup(target.c_str());
  if (!args[1]) {
    free(args[0]);
    free(args);
    Logger::error(__FILE__, "Allocating memory for args[1] failed");
    response->setStatusCode(500);
    response->setBody(getErrorPage(response->getStatus(), client->getServer()));
    return;

    return;
  }
  args[2] = NULL;
}

// https://datatracker.ietf.org/doc/html/rfc3875#section-4.1
char **Cgi::generateEnv(Client *client, std::string exec_path) {
  Request *request = client->getRequest();
  Response *response = client->getResponse();
  if (!request || !response)
    return NULL;

  std::vector<std::string> env_vec;

  env_vec.push_back("AUTH_TYPE=Basic");
  env_vec.push_back("GATEWAY_INTERFACE=CGI/1.1");
  env_vec.push_back("PATH_INFO=" + exec_path);
  env_vec.push_back("PATH_TRANSLATED=" + exec_path);
  env_vec.push_back("REQUEST_URI=" + exec_path);
  env_vec.push_back("SERVER_SOFTWARE=URMOM");
  env_vec.push_back("QUERY_STRING=" + request->getQueryParam());
  env_vec.push_back("REQUEST_METHOD=" + request->getMethod());
  env_vec.push_back("SCRIPT_FILENAME=" + exec_path);
  env_vec.push_back("SCRIPT_NAME=" + exec_path);
  env_vec.push_back("SERVER_PROTOCOL=HTTP/1.1");

  if (request->getMethod() == "POST") {
    std::map<std::string, std::string> &headers = request->getHeaders();
    std::string cl =
        headers.count("content-length") ? headers["content-length"] : "";
    std::string ct =
        headers.count("content-type") ? headers["content-type"] : "";
    env_vec.push_back("CONTENT_LENGTH=" + cl);
    env_vec.push_back("CONTENT_TYPE=" + ct);
  }

  char **envp = (char **)calloc(env_vec.size() + 1, sizeof(char *));
  if (!envp) {
    Logger::error(__FILE__, "Allocating memory for envp failed");
    return NULL;
  }

  for (size_t i = 0; i < env_vec.size(); ++i) {
    envp[i] = strdup(env_vec[i].c_str());
    if (!envp[i]) {
      for (size_t j = 0; j < i; ++j)
        free(envp[j]);
      free(envp);
      return NULL;
    }
  }

  return envp;
}

void Cgi::execute(Client *client) {

    Request *request = client->getRequest();
    Response *response = client->getResponse();
    if (!request || !response)  
        return;

  if (pipe(this->pipe_in) < 0) {
    Logger::error(__FILE__, "pipe() failed");
    response->setStatusCode(500);
    response->setBody(getErrorPage(response->getStatus(), client->getServer()));
    return;
  }
  if (pipe(this->pipe_out) < 0) {
    Logger::error(__FILE__,  "pipe() failed");
    close(pipe_in[0]);
    close(pipe_in[1]);
    response->setStatusCode(500);
    response->setBody(getErrorPage(response->getStatus(), client->getServer()));
    return;
  }
  this->cgi_pid = fork();
  if (this->cgi_pid == 0) {
    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    int _exit_status = execve(this->args[0], this->args, this->envp);
    exit(_exit_status);
  } else if (this->cgi_pid > 0) {
  } else {
    response->setStatusCode(500);
    response->setBody(getErrorPage(response->getStatus(), client->getServer()));
  }
}

char **Cgi::getArgs() { return this->args; }

void Cgi::setEnv(char **envp) {
  if (this->envp) {
    freeEnv(this->envp);
  }
  this->envp = envp;
}

char **Cgi::getEnv() const { return this->envp; }

void Cgi::freeEnv(char **envp) {
  if (!envp)
    return;
  for (size_t i = 0; envp[i] != NULL; ++i)
    free(envp[i]);
  free(envp); // Use free since calloc was used
}

void Cgi::setCGIState(int state) { this->cgi_state = state; }

int Cgi::getCGIState() { return this->cgi_state; }

int *Cgi::getPipeIn() { return this->pipe_in; }

int *Cgi::getPipeOut() { return this->pipe_out; }

int Cgi::getCgiPid() { return this->cgi_pid; }


void Cgi::reset() {
  if (this->args) {
    for (size_t i = 0; this->args[i] != NULL; ++i)
      free(this->args[i]);
    free(this->args);
  }
  if (this->envp) {
    freeEnv(this->envp);
  }
  this->args = NULL;
  this->envp = NULL;
}
Cgi::Cgi(){};
Cgi::~Cgi() {}
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
