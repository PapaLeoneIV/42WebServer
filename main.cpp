
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>




// PREPROCESS:
//     rimuovere i commenti
//     rimuovere le righe vuote

enum STATES{
    WAITING_STATE,
    SERVER_DIRECTIVE_STATE,
    LOCATION_DIRECTIVE_STATE,
    STATE4,
    STATE5
};


// Server{
//     serverDirectives: [listen:8001][host:120.0.0.1] ...
//     locations{
//         /tours[root docs/fusion_web][autoindex on] ...
//         /cgi-bin[][][]
//         /images[][][]
//     }
// }
// Server{
//     serverDirectives: [listen:8001][host:120.0.0.1] ...
//     locations{
//         /tours[root docs/fusion_web][autoindex on] ...
//         /cgi-bin[][][]
//         /images[][][]
//     }
// }

class Server{
    public:
    std::map<std::string, std::vector<std::string> > directives;
    std::map<std::string, std::map<std::string, std::vector<std::string> > > locations;

};


std::string trim(const std::string& str) {
    bool isOnlySpaces = true;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (*it != ' ') {
            isOnlySpaces = false;
            break;
        }
    }
    if (isOnlySpaces)
        return "";
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return str;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

int main(){ 
    std::ifstream file("./nginx.conf");
    if (!file) {
        std::cerr << "Unable to open file nginx.conf" << std::endl;
        return 1;
    }

    std::vector<Server*> serverVector;
    bool serverOpen = false;
    bool locationOpen = false;

    std::string locationPath;
    int serverCounter = 0;

    std::string line;
    int state = WAITING_STATE;
    while (std::getline(file, line)) {
        //rimuovere i commenti
        line = line.substr(0, line.find("#"));
        //rimuovere le righe vuote
        line = trim(line);
        if(line.empty()){continue;}

        switch (state){
        case WAITING_STATE: // waiting state
            if(line.find("}") != std::string::npos){
                break;
            }
            if(line == "server {"){
                //create new server
                serverOpen = true;
                Server *server = new Server();
                serverVector.push_back(server);
                serverCounter++;
                state = SERVER_DIRECTIVE_STATE;
                continue;
            }
            throw std::runtime_error("Errore di parsing: server { non trovato");
        case SERVER_DIRECTIVE_STATE: // server directives
            if(line.find("server {") != std::string::npos && serverOpen){
                throw std::runtime_error("Errore di parsing: server { non chiuso");
            }

            if(line.find(";") != std::string::npos){
                std::string key = trim(line.substr(0, line.find(" ")));
                //TODO handle multiple values
                //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/4
                std::string value = trim(line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1));
                serverVector[serverCounter - 1]->directives[key].push_back(value);
                continue;
            }
            if(line.back() == '{' && line.find("location") != std::string::npos){
                locationOpen = true;
                locationPath = trim(line.substr(line.find("location") + 8,
                                                 line.find("{") - line.find("location") - 8));
                //create entry into map of location using locationPath as key
                serverVector[serverCounter - 1]->locations[locationPath] = std::map<std::string, std::vector<std::string> >();
                state = LOCATION_DIRECTIVE_STATE;
                continue;
            }
             if(line.find("}") != std::string::npos){
                serverOpen = false;
                state = WAITING_STATE;
                continue;
            }
        
        case LOCATION_DIRECTIVE_STATE: // location directives
            if(line.back() == '{' && line.find("location") != std::string::npos && locationOpen){
                throw std::runtime_error("Errore di parsing: location { non chiuso");
            }
            if(line.find(";") != std::string::npos){
                std::string key = trim(line.substr(0, line.find(" ")));
                //TODO handle multiple values
                //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/3
                std::string value = trim(line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1));
                //add entry into map of location
                serverVector[serverCounter - 1]->locations[locationPath][key].push_back(value);
                continue;
            }
            if(line.back() == '}'){
                locationOpen = false;
                state = SERVER_DIRECTIVE_STATE;
                continue;
            }
        default:
            continue;
        }

    }

    for(std::vector<Server*>::iterator it = serverVector.begin(); it != serverVector.end(); ++it){
        Server* server = *it;
        std::cout << "Server: " << std::endl;
        for(std::map<std::string, std::vector<std::string> >::iterator dirIt = server->directives.begin(); dirIt != server->directives.end(); ++dirIt){
            std::cout << "  " <<dirIt->first << " : ";
            for(std::vector<std::string>::iterator valIt = dirIt->second.begin(); valIt != dirIt->second.end(); ++valIt){
                std::cout << *valIt << " ";
            }
            std::cout << std::endl;
        }
        std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator locationMapIterator = server->locations.begin();
        for(; locationMapIterator != server->locations.end(); ++locationMapIterator){
            std::string path = locationMapIterator->first;
            std::cout << "      Location: " << path << std::endl;
            std::map<std::string, std::vector<std::string> >::iterator ValueVector = locationMapIterator->second.begin();
            for(; ValueVector != locationMapIterator->second.end(); ++ValueVector){
                std::cout << "          " << ValueVector->first << " : ";
                std::vector<std::string>::iterator Value = ValueVector->second.begin();
                for(; Value != ValueVector->second.end(); ++Value){
                    std::cout << *Value;
                }
                std::cout << std::endl;
            }
        }
    }
    file.close();
    }