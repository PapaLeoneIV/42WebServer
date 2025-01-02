#include "Server.hpp"
#include "Booter.hpp"

int main()
{

    ERROR error;
    Server server;
    Booter booter;

    try {
        if((error = booter.bootServer(&server, "localhost", "6968"))){
            //TODO print error
            return 1;
        }
    }
    catch(const std::exception& e) { std::cerr << "Error: booting process failed\n" << e.what() << '\n'; return(1);}
    try
    {
        for(;server._keep_alive;)
        {
            if((error = server.handleConnections())){
                throw std::runtime_error("Error: handling connections failed\n");
            }
        }
    }
    catch(const std::exception& e) { std::cerr << "error: handling connections failed" << e.what() << '\n';}
    return 0;
}


        