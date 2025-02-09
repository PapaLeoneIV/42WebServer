#include <cctype>
#include <ios>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
enum states{
    Method_state,
    Post_Or_Put_state,
    Space_After_method,
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
    headers_key_state,
    headers_trailing_space_start,
    headers_value,
    headers_trailing_space_end,
    headers_CR,
    headers_LF,
    headers_end_CR,
    headers_end_LF, 
    body_start_state,
    body_plain_text,
    body_chunked_text,
    body_chunked_number,
    body_end_CR,
    body_end_LF,
    chunked_end_LF,
    chunked_number_LF,
    chunked_number_CR,
    chunked_chunk_LF,
    chunked_chunk, 
    parsing_complete,
  
  };
  
  enum methods{
    GET,
    POST,
    DELETE,
  };

  int strToInt(std::string str) {
    std::stringstream ss(str);
    int number;
    ss >> number;
    return number;
}

  std::string to_lower(const std::string& input) {
    std::string result = input; // Make a copy of the input string
    for (std::string::size_type i = 0; i < result.size(); ++i) {
        result[i] = std::tolower(static_cast<unsigned char>(result[i])); // Convert to lowercase
    }
    return result;
}


typedef struct parser{
    std::map<int, std::string> methods;
    std::map<std::string, std::string> headers;
    std::string buffer;
    int state;
    std::string method;
    std::string url;
    std::string version;
    std::string headers_key;
    std::string body;
    std::string content;
    bool has_body;
    bool is_chunked;
    int body_counter;
    int error;
    std::string raw;
    size_t counter;
    size_t number;
} parserr ;

void print_parser(parser& p) {
    // Print the methods map
    // Print the headers map
    // std::cout << "Headers:" << std::endl;
    // for (std::map<std::string, std::string>::const_iterator it = p.headers.begin(); it != p.headers.end(); ++it) {
    //     std::cout << "  " << it->first << ": " << it->second << std::endl;
    // }

    // Print other fields
  /*   std::cout << "Buffer: " << p.buffer << std::endl;
    std::cout << "State: " << p.state << std::endl;
    std::cout << "Method: " << p.method << std::endl;
    std::cout << "URL: " << p.url << std::endl;
    std::cout << "Version: " << p.version << std::endl;
    std::cout << "Headers Key: " << p.headers_key << std::endl;
    std::cout << "Body: " << p.body << std::endl;
    std::cout << "Content: " << p.content << std::endl;
    std::cout << "Has Body: " << (p.has_body ? "true" : "false") << std::endl;
    std::cout << "Is Chunked: " << (p.is_chunked ? "true" : "false") << std::endl;
    std::cout << "Body Counter: " << p.body_counter << std::endl;
    std::cout << "Error: " << p.error << std::endl; */
    std::cout << "Raw: " << p.raw << std::endl;
}


int consume(std::string buffer, parserr *p){
    for(size_t i = 0; i < buffer.size(); i++){
        char character = buffer[i];
        switch(p->state){
            case Method_state: {
                if(character == p->methods[GET][p->raw.size()]){
                    p->content += character;
                    p->raw += character;
                    if(p->raw.size() == p->methods[GET].size())
                    {
                        p->method = p->content;
                        p->content.clear();
                        if(p->method != p->methods[GET])
                        {
                            p->error = -1;
                            return 1;
                        }
                        p->state = Space_After_method;
                        continue;
                    }
                    continue;
                }
                if(character == p->methods[POST][p->raw.size()]){
                    p->content += character;
                    p->raw += character;
                    if(p->raw.size() == p->methods[POST].size())
                    {
                        p->method = p->content;
                        p->content.clear();
                        if(p->method != p->methods[POST])
                        {
                            p->error = -1;
                            return 1;
                        }
                        p->state = Space_After_method;
                        continue;
                    }
                    
                    continue;
                }
                if(character == p->methods[DELETE][p->raw.size()]){
                    p->raw += character;
                    p->content += character;
                    if(p->raw.size() == p->methods[DELETE].size())
                    {
                        p->method = p->content;
                        p->content.clear();
                        if(p->method != p->methods[DELETE])
                        {
                            p->error = -1;
                            return 1;
                        }
                        p->state = Space_After_method;
                        continue;
                    }
                    p->content += character;
                    continue;
                }
                p->error = -1; //invalid method
                return 1;   
            }
            case Space_After_method: {
                if(character != ' ')
                {
                    p->error = -1; //request not valid
                    return 1;
                }
                p->state =  Url_begin_state;
                break;
            }
            case Url_begin_state:{
                if(character != '/')
                {
                    p->error = -1; //request not valid
                    return 1;
                }
                p->content.clear();
                p->state =  Url_string_state;
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
                    break;
                }

                //% https://datatracker.ietf.org/doc/html/rfc3986#section-2.1 HTTP/1.1
                if(character == '%'){
                    // TODO :
                    //switch to state "encoded percent"
                    break;
                }
                
                if(character == ' ')
                {
                    p->state = Space_after_url;
                    break;
                } 
                break;
            }
            case Space_after_url: {
                if(character != 'H'){
                    p->error = -1; //request not valid
                    return 1;
                }
                p->url = p->content;
                if(p->url.size() > 4 * 1024){ //url cannot be longer than 4MB 
                    p->error = -1;
                    return 1;
                }
                p->content.clear();
                p->state =  Http_version_state_H;
                break;
            }
            //"HTTP/1.1 only version admitted
            case Http_version_state_H:{
                if(character != 'T'){
                    p->error = -1;
                    return 1;
                }
                p->state = Http_version_state_HT;
                break;
            }
            case Http_version_state_HT:{
                if(character != 'T'){
                    p->error = -1;
                    return 1;
                }
                p->state = Http_version_state_HTT;
                break;
            }
            case Http_version_state_HTT:{
                if(character != 'P'){
                    p->error = -1;
                    return 1;
                }
                p->state = Http_version_state_HTTP;
                break;
            }
            case Http_version_state_HTTP:{
                if(character != '/'){
                    p->error = -1;
                    return 1;
                }
                p->state = Http_version_state_HTTP_sep;
                break;
            }
            case Http_version_state_HTTP_sep:{
                if(character != '1'){
                    p->error = -1;
                    return 1;
                }
                p->state = Http_version_state_HTTP_sep_one;
                break;
            }
            case Http_version_state_HTTP_sep_one:{
                if(character != '.'){
                    p->error = -1;
                    return 1;
                }
                p->state = Http_version_state_HTTP_sep_one_dot;
                break;
                
            }
            case Http_version_state_HTTP_sep_one_dot:{
                if(character != '1'){   
                    p->error = -1;
                    return 1;
                }
                p->state = Http_version_state_HTTP_sep_one_dot_one;
                break;
            }
            case Http_version_state_HTTP_sep_one_dot_one:{
                if(character != '\r'){
                    p->error = -1;
                    return 1;
                }
                p->version = p->content;
                p->content.clear();
                p->state = firstline_CR;
                break;
            }
            case firstline_CR:{
                if(character != '\n'){
                    p->error = -1;
                    return 1;
                }
                p->content.clear();
                p->state = headers_key_state;
                p->raw += character;
                continue;
            }
            //HTTP headers are structured such that each header field consists of a case-insensitive field name followed by a colon (:),
            // optional leading whitespace, the field value, and optional trailing whitespace.They are serialized into a single string where 
            // individual header fields are separated by CRLF (carriage return 1 and line feed, represented by \r\n in many programming languages).
            case headers_key_state: {
                if(character == ':')
                {
                    p->headers_key = p->content;
                    p->content.clear();
                    p->state = headers_trailing_space_start;
                    break;
                }
                if(character == ' ') //no spaces allowed between header key and column
                {
                    p->error = -1;
                    return 1;
                }
                if((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')  
                || (character >= '0' && character <= '9') || (character == '_') || (character == '-')){
                    break;
                }
                std::cout << "p->error: character not allowed" << std::endl;
                p->error = -1;
                return 1;
            }
            case headers_trailing_space_start: {
                if(character == ' ')
                {
                    break;
                }
                p->content.clear();
                p->state = headers_value;
                break;
            }
            case headers_value: {
                
                if(character == ' ')
                {
                    //std::cout << "HEADERS MAP INSERTING KEY VAL" << std::endl;
                    //std::cout << headers_key << " " << p->content << std::endl;
                    p->headers[to_lower(p->headers_key)] = to_lower(p->content);

                    p->content.clear();
                    //sono rimasto qui
                    p->state = headers_trailing_space_end;
                    break;
                }
                if(character == '\r')
                {
                    //std::cout << "HEADERS MAP INSERTING KEY VAL" << std::endl;
                    //std::cout << headers_key << " " << p->content << std::endl;
                    p->headers[to_lower(p->headers_key)] = to_lower(p->content);
                    p->content.clear();
                    p->raw += character;
                    p->state = headers_CR;
                    continue;
                }
                if(character < 32 || character > 126) //non printable chars
                {
                    p->error = -1;
                    return 1;
                }
                //  //sep = "()<>@,;:\\\"/[]?={} \t"
                // if(character == '(' || character == ')' || character == '<' || character == '>'
                // || character == '@' || character == ',' || character == ';' || character == ':'
                // || character == '\\' || character == '\"' /* || character == '/' */ || character == '['
                // || character == ']' || character == '?' || character == '=' || character == '{'
                // || character == '}')
                // {
                //     p->error = -1;
                //     return 1;
                // }
                break;
            
            }
            case headers_trailing_space_end: {
                if(character == ' '){
                    break;
                }
                if(character == '\r'){
                    p->state = headers_CR;
                    break;
                }
                p->state = headers_value;
                break;
            }
            case headers_CR:{
                if(character == '\n'){
                    p->state = headers_LF;
                    p->raw += character;
                    continue;
                }
                p->error = -1;
                return 1;
            }
            case headers_LF:{
                if(character == '\r')
                {
                    p->state = headers_end_CR;
                    p->raw += character;
                    continue;
                }
                p->state = headers_key_state;
                break;
            }
            case headers_end_CR:{
                if(character != '\n')
                {
                    p->error= -1;
                    return 1;
                }
                if(!p->headers["content-length"].empty())
                {
                    p->has_body = true;
                    p->is_chunked = false;
                }
                else if(!p->headers["transfer-encoding"].empty())
                {
                    p->has_body = true;
                    p->is_chunked = true;
                }
                if(p->has_body && p->method == "POST"){
                    if(p->is_chunked)
                        p->state = body_chunked_number;
                    else
                        p->state = body_plain_text;
                    p->raw += character;
                    p->content.clear();
                    continue;
                }
                p->raw += character;
                p->content.clear();
                p->state = parsing_complete;
                continue;
            }
            case body_plain_text:{
                if(p->body_counter < strToInt(p->headers["content-length"]))
                {
                    p->body_counter++;
                    p->body += character;
                    p->raw+=character;
                    continue;
                }
                if(p->body_counter == strToInt(p->headers["content-length"]))
                {
                    //TODO check if this raw is needed
                    p->state = parsing_complete;
                    continue;
                }
                break;
            }
            case body_chunked_number:{
                if(character == '\r')
                {
                    std::stringstream ss;
                    ss << std::hex << p->content;
                    ss >> p->number;
                    if(p->number == 0)
                    {
                        p->content.clear();
                        p->state = chunked_end_LF;
                        continue;
                    }
                    p->content.clear();
                    p->raw += character;
                    p->state = chunked_number_LF;
                    continue;
                }
                break;
            }
            case chunked_number_CR : {
                if(character != '\r')
                {
                    p->error= -1;
                    return 1;

                }
                p->state = chunked_chunk_LF;
                continue;
            }
            case chunked_number_LF : {
                if(character != '\n'){
                    p->error= -1;
                    return 1;

                }
                p->state = chunked_chunk;
                continue;
            }
            case chunked_chunk : {  
                if(p->content.size() < p->number)
                {
                    p->content += character;
                    p->raw += character;
                    continue;
                }
                if(character == '\r')
                {
                    p->body += p->content;
                    p->raw += character;
                    p->content.clear();
                    p->state = chunked_chunk_LF;
                    continue;
                }
                break;
            }
            case chunked_chunk_LF : {  
                if(character != '\n')
                {
                    p->error = -1;
                    return 1;
                    
                }
                p->raw += character;
                p->state = body_chunked_number;
                continue;
            }
            case chunked_end_LF:{
                if(character != '\n')
                {
                    p->error = -1;
                    return 1;
                }
                //TODO understand if i need to handle an additional CRLF at the end of the body, after the 0/r/n
                p->raw += character;
                p->state = parsing_complete;
            }
        
            case parsing_complete:{
                //print_parser(*p);

                return 0;
            }
            

            default:
                std::cout << "case not handled ==> " << character << std::endl;
        }
        p->raw += character;
        p->content += character;
    }

    return 1;
}

int main(){

    parserr parser;
    parser.has_body = false;
    parser.is_chunked = false;
    parser.state = Method_state;

    parser.body_counter = 0;
    parser.error = 0;

    std::string buffer = "POST /upload HTTP/1.1\r\nHost:";
    std::string buffer2 = " example.com\r\nContent-Length: 12\r\n\r\nThis is the\r\n";
    // std::string buffer2 = "ET /example HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/html\r\n\r\n";
    parser.methods.insert(std::make_pair(GET, "GET"));
    parser.methods.insert(std::make_pair(POST, "POST"));
    parser.methods.insert(std::make_pair(DELETE, "DELETE"));


    //https://datatracker.ietf.org/doc/html/rfc7230#section-3

    consume(buffer, &parser);
    print_parser(parser);

    std::cout << "Second Request start from here: " << std::endl;
    consume(buffer2, &parser);

    print_parser(parser);


    //std::map<std::string, std::string>::iterator it;
    //// for(it = headers.begin(); it != headers.end(); ++it){
    //     std::cout << "Key: " << it->first << " Value: " << it->second << std::endl;
    // }
    //std::cout << content << std::endl;
    //std::cout << raw << std::endl;

    (void)parser.error;
}