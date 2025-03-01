
#include <exception>
#include <string>

class Exception : public std::exception {
    private:
    std::string message;
    
    public:
        Exception(const char* msg) : message(msg) {}
        Exception(const std::string& msg) : message(msg) {}
        virtual ~Exception() throw() {}
    
        const char* what() const throw() {
            return message.c_str();
        }
};