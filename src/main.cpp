#include "Server.hpp"
#include <iostream>
#include "stdlib.h"
#include <signal.h>


void signal_handler(int signal)
{
    std::cout << "Signal received: " << signal << std::endl;
    Server server;
    server.keep_alive = false;
}


int main()
{

    ERROR error;
    Server server;

    signal(SIGINT, signal_handler);
    try {
        if((error = server.boot_server("localhost", "6969")))
        {
            //TODO print error
            throw std::runtime_error("Error: booting process failed\n"); ;
        }
    }
    catch(const std::exception& e) { std::cerr << "Error: booting process failed" << e.what() << '\n'; exit(1);}
    try
    {
        for(;server.keep_alive;)
        {
            if((error = server.handle_connections()))
            {
                throw std::runtime_error("Error: handling connections failed\n");
            }
        }
    }
    catch(const std::exception& e) { std::cerr << "error: handling connections failed" << e.what() << '\n';}
    return 0;
}


        