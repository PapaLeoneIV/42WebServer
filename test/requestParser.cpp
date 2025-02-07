#include <map>
#include <string>
enum states{
    Method_state,
    Post_or_Put_state,
    Space_after_method,
    Url_begin_state,
    Url_string_state,
    Space_after_url,
    Http_version_state,
    Http_version_state_H,
    Http_version_state_HT,
    Http_version_state_HTT,
    Http_version_state_HTTP,
    Http_version_state_HTTP_sep,
    Http_version_state_HTTP_sep_one,
    Http_version_state_HTTP_sep_one_dot,
    Http_version_state_HTTP_sep_one_dot_one,
    firstline_CR,
    firstline_LF,
    header_start,
    headers_trailing_space_start,
    headers_value,
    headers_trailing_space_end,
    headers_CR,
    headers_LF,
  
  };
  
  enum methods{
    GET,
    POST,
    DELETE,
    PUT,
    HEAD,
  };

int main(){
    std::map<int, std::string> methods;
    std::string buffer;
    int state;
    std::string method;

    methods.insert(std::make_pair(GET, "GET"));
    methods.insert(std::make_pair(POST, "POST"));
    methods.insert(std::make_pair(DELETE, "DELETE"));
    methods.insert(std::make_pair(PUT, "PUT"));
    methods.insert(std::make_pair(HEAD, "HEAD"));


    //https://datatracker.ietf.org/doc/html/rfc7230#section-3
    std::string content;
    state = Method_state;
    int error;

    buffer = "GET / HTTP/1.1\r\n" \
             "headers:    value   \r\n";

    for(size_t i = 0; i < buffer.size(); i++){
        char character = buffer[i];
        
        
        switch(state){
            case Method_state: {
                if(character == 'G'){
                    method = "GET";
                    content.append("GET");
                    i += 2;
                    state = Space_after_method;
                    continue;
                }
                else if(character == 'P'){
                    state = Post_or_Put_state;
                    break;
                }
                else if (character == 'D'){
                    method = "DELETE";
                    content.append("DELETE");
                    i += 4;
                    state = Space_after_method;
                    continue;
                } else {
                    error = -1; //invalid method
                    return 1;
                }   
            }
            case Post_or_Put_state: {
                if(character == 'O'){
                    method = "POST";
                    content.append("OST");
                    state = Space_after_method;
                    i += 3;
                    break;
                }
                else if (character == 'U')
                {
                    method = "PUT";
                    content.append("UT");
                    state = Space_after_method;
                    i += 2;
                    break;
                }
                else{
                    error = -1; //invalid method
                    return 1;
                }
            }
            case Space_after_method: {
                if(character != ' ')
                {
                    error = -1; //request not valid
                    return 1;
                }
                state = Url_begin_state;
                break;
            }
            case Url_begin_state:{
                if(character != '/')
                {
                    error = -1; //request not valid
                    return 1;
                }
                state =  Url_string_state;
                break;
            }
            case Url_string_state:{
                // https://datatracker.ietf.org/doc/html/rfc1738#section-2.1
                if((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')  
                || (character >= '0' && character <= '9') || (character == '+') || (character == '.')
                || (character == '-') || (character == '_') || (character == '!') || (character == '$')  
                || (character == '*') || (character == '\'') || (character == '(') || (character == ')')
                || (character == '/')){
                    break;
                }
                //TODO '?' 
                if(character == '?'){
                    //TODO :
                    //switch to state "extract query params"
                }

                //% https://datatracker.ietf.org/doc/html/rfc3986#section-2.1 HTTP/1.1
                if(character == '%'){
                    // TODO :
                    //switch to state "encoded percent"
                }
                
                if(character == ' ')
                {
                    state = Space_after_url;
                    break;
                }
                
            }
            case Space_after_url: {
                if(character != 'H'){
                    error = -1; //request not valid
                    return 1;
                }
                state =  Http_version_state_H;
                break;
            }
            //"HTTP/1.1 only version admitted
            case Http_version_state_H:{
                if(character != 'T'){
                    error = -1;
                    return 1;
                }
                state = Http_version_state_HT;
                break;
            }
            case Http_version_state_HT:{
                if(character != 'T'){
                    error = -1;
                    return 1;
                }
                state = Http_version_state_HTT;
                break;
            }
            case Http_version_state_HTT:{
                if(character != 'P'){
                    error = -1;
                    return 1;
                }
                state = Http_version_state_HTTP;
                break;
            }
            case Http_version_state_HTTP:{
                if(character != '/'){
                    error = -1;
                    return 1;
                }
                state = Http_version_state_HTTP_sep;
                break;
            }
            case Http_version_state_HTTP_sep:{
                if(character != '1'){
                    error = -1;
                    return 1;
                }
                state = Http_version_state_HTTP_sep_one;
                break;
            }
            case Http_version_state_HTTP_sep_one:{
                if(character != '.'){
                    error = -1;
                    return 1;
                }
                state = Http_version_state_HTTP_sep_one_dot;
                break;
                
            }
            case Http_version_state_HTTP_sep_one_dot:{
                if(character != '1'){   
                    error = -1;
                    return 1;
                }
                state = Http_version_state_HTTP_sep_one_dot_one;
                break;
            }
            case Http_version_state_HTTP_sep_one_dot_one:{
                if(character != '\r'){
                    error = -1;
                    return 1;
                }
                state = firstline_CR;
                break;
            }
            case firstline_CR:{
                if(character != '\n'){
                    error = -1;
                    return 1;
                }
                state = firstline_LF;
                break;
            }
            case firstline_LF:{
                if(character == '\n'){
                    state = header_start;
                    break;
                }
                else{
                    error = -1;
                    return 1;
                }
            }
            //HTTP headers are structured such that each header field consists of a case-insensitive field name followed by a colon (:),
            // optional leading whitespace, the field value, and optional trailing whitespace.They are serialized into a single string where 
            // individual header fields are separated by CRLF (carriage return 1 and line feed, represented by \r\n in many programming languages).
            case header_start: {
                if(character == ':')
                {
                    state = headers_trailing_space_start;
                    break;
                }
                if(character == ' ')
                {
                    error = -1;
                    return 1;
                }
                break;
            }
            case headers_trailing_space_start: {
                if(character == ' ')
                {
                    break;
                }
                state = headers_value;
                break;
            }
            case headers_value: {
                if(character == ' ')
                {
                    state = headers_trailing_space_end;
                    break;
                }
                state = headers_value;
                break;
            
            }
            case headers_trailing_space_end: {
                if(character == ' '){
                    break;
                }
                if(character == '\r'){
                    state = headers_CR;
                }
                state = headers_value;
                break;
            }
            case headers_CR:{
                if(character != '\n'){
                    error = -1;
                    return 1;
                }
            }
            
            content += character;
        }
    }
}


