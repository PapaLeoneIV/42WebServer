#include "Utils.hpp"


enum STATE{
    NORMAL_STATE,
    SERVER_STATE,
    LOCATION_STATE,
};

int main(){
    bool bracketOpen = false;
    bool bracketClose = false;
    
    std::string input;
    char c;
    int index;
    int len = input.length();
    int state = NORMAL_STATE;
    int i = 0;
    while(i < len)
    {
        c = input[i];
    }
}