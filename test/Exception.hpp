
#include <exception>
#include <string>

class Exception : public std::exception {
    private:
    std::string message;
    
    public:
        Exception(const char* msg) : message(msg) {}
        ~Exception() throw() {}
    
        const char* what() const throw() {
            return message.c_str();
        }
};