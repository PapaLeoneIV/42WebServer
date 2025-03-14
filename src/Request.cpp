#include "Request.hpp"
#include "Utils.hpp"

/**
* After we user recv(), we pass the request to consume().
* It will analyze the request character by character, trying to 
* respect the HTTP protocol standard for request firstline, headers and body.
* While doing so, it keeps track of the various states of the request and where it 
* did stop parsing. So that, it the request is chunked, or
* if the Client sended just a portion of the request, and will send the next one later, 
* consume() will be able to resume parsing where it previously stopped.
*/

// TODO: per il momento ho temporaneamente settato error a -1
// Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/23
// bisogna settarlo ai vari error codes, 400 bad request e cosi via
// PS: se guardi in 'Utils.hpp', nella funzione getMessageFromStatusCode(int status) trovi alcuni degl errori da gestire
int Request::consume(std::string buffer){
    for(size_t i = 0; i < buffer.size(); i++){
        char character = buffer[i];
        switch(this->state){
            case StateMethod: {
                if(character == this->methods[GET][this->raw.size()]){
                    this->content += character;
                    this->raw += character;
                    if(this->raw.size() == this->methods[GET].size())
                    {
                        this->method = this->content;
                        this->content.clear();
                        if(this->method != this->methods[GET])
                        {
                            this->error = 400; //Bad Request
							this->state = StateParsingError;
                            return 1;
                        }
                        this->state = StateSpaceAfterMethod;
                        continue;
                    }
                    continue;
                }
                if(character == this->methods[POST][this->raw.size()]){
                    this->content += character;
                    this->raw += character;
                    if(this->raw.size() == this->methods[POST].size())
                    {
                        this->method = this->content;
                        this->content.clear();
                        if(this->method != this->methods[POST])
                        {
                            this->error = 400; //Bad Request
							this->state = StateParsingError;
                            return 1;
                        }
                        this->state = StateSpaceAfterMethod;
                        continue;
                    }
                    continue;
                }
                if(character == this->methods[DELETE][this->raw.size()]){
                    this->raw += character;
                    this->content += character;
                    if(this->raw.size() == this->methods[DELETE].size())
                    {
                        this->method = this->content;
                        this->content.clear();
                        if(this->method != this->methods[DELETE])
                        {
                            this->error = 400; //Bad Request
							this->state = StateParsingError;
                            return 1;
                        }
                        this->state = StateSpaceAfterMethod;
                        continue;
                    }
                    this->content += character;
                    continue;
                }
                this->error = 501; //invalid method(Not Implemented)
				this->state = StateParsingError;
                return 1;   
            }
            case StateSpaceAfterMethod: {
                if(character != ' ')
                {
                    this->error = 400; //request not valid
					this->state = StateParsingError;
                    return 1;
                }
                this->state =  StateUrlBegin;
                break;
            }
            case StateUrlBegin:{
                if(character != '/')
                {
                    this->error = 400; //request not valid
					this->state = StateParsingError;
                    return 1;
                }
                this->content.clear();
                this->state =  StateUrlString;
                break;
            }
            case StateUrlString:{
                // https://datatracker.ietf.org/doc/html/rfc1738#section-2.1
                if((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')  
                || (character >= '0' && character <= '9') || (character == '+') || (character == '.')
                || (character == '-') || (character == '_') || (character == '!') || (character == '$')  
                || (character == '*') || (character == '\'') || (character == '(') || (character == ')')
                || (character == '/')){
                    break;
                }
                //TODO '?' handle query params if we want to do it
                //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/10
                if(character == '?'){
                    //switch to state "extract query params"
                    break;
                }

                //% https://datatracker.ietf.org/doc/html/rfc3986#section-2.1 HTTP/1.1
                if(character == '%'){
                    this->raw += character;
                    this->state = StateEncodedSep;
                    // TODO :
                    // Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/9
                    //switch to state "encoded percent"
                    continue;
                }
                
                if(character == ' ')
                {
                    this->raw += character;
                    this->state = StateSpaceAfterUrl;
                    continue;
                } 
                break;
            }
            case StateEncodedSep:{
                if(this->encoded_counter == 1)
                {
                    this->encoded_char += character;
                    this->raw += character;

                    int hex = strToHex(this->encoded_char);
                    this->encoded_char = static_cast<char>(hex);
                   
                    this->content += this->encoded_char;

                    this->encoded_counter = 0;
                    this->encoded_char.clear();
                    this->state = StateUrlString;
                    continue;
                }
                this->encoded_char += character;
                this->raw += character;
                this->encoded_counter++;
                continue;
            }

            case StateSpaceAfterUrl: {
                if(character != 'H'){
                    this->error = 400; //request not valid
					this->state = StateParsingError;
                    return 1;
                }
                this->url = this->content;
                if(this->url.size() > 4 * 1024){ //url cannot be longer than 4MB 
                    this->error = 414; //URI too long
					this->state = StateParsingError;
                    return 1;
                }
                this->content.clear();
                this->state =  StateVersion_H;
                break;
            }
            //"HTTP/1.1 only version admitted
            case StateVersion_H:{
                if(character != 'T'){
                    this->error = 505; //HTTP Version Not Supported
					this->state = StateParsingError;
                    return 1;
                }
                this->state = StateVersion_HT;
                break;
            }
            case StateVersion_HT:{
                if(character != 'T'){
                    this->error = 505;
					this->state = StateParsingError;
                    return 1;
                }
                this->state = StateVersion_HTT;
                break;
            }
            case StateVersion_HTT:{
                if(character != 'P'){
                    this->error = 505;
					this->state = StateParsingError;
                    return 1;
                }
                this->state = StateVersion_HTTP;
                break;
            }
            case StateVersion_HTTP:{
                if(character != '/'){
                    this->error = 505;
					this->state = StateParsingError;
                    return 1;
                }
                this->state = StateVersion_HTTPSlash;
                break;
            }
            case StateVersion_HTTPSlash:{
                if(character != '1'){
                    this->error = 505;
					this->state = StateParsingError;
                    return 1;
                }
                this->state = StateVersion_HTTPSlashOne;
                break;
            }
            case StateVersion_HTTPSlashOne:{
                if(character != '.'){
                    this->error = 505;
					this->state = StateParsingError;
                    return 1;
                }
                this->state = StateVersion_HTTPSlashOneDot;
                break;
                
            }
            case StateVersion_HTTPSlashOneDot:{
                if(character != '1'){   
                    this->error = 505;
					this->state = StateParsingError;
                    return 1;
                }
                this->state = StateVersion_HTTPSlashOneDotOne;
                break;
            }
            case StateVersion_HTTPSlashOneDotOne:{
                if(character != '\r'){
                    this->error = 400; //Bad Request
					this->state = StateParsingError;
                    return 1;
                }
                this->version = this->content;
                this->content.clear();
                this->state = StateFirstLine_CR;
                break;
            }
            case StateFirstLine_CR:{
                if(character != '\n'){
                    this->error = 400;
					this->state = StateParsingError;
                    return 1;
                }
                this->content.clear();
                this->state = StateHeaderKey;
                this->raw += character;
                continue;
            }
            //HTTP headers are structured such that each header field consists of a case-insensitive field name followed by a colon (:),
            // optional leading whitespace, the field value, and optional trailing whitespace.They are serialized into a single string where 
            // individual header fields are separated by CRLF (carriage return 1 and line feed, represented by \r\n in many programming languages).
            case StateHeaderKey: {
                if(character == ':')
                {
                    this->headers_key = this->content;
                    this->content.clear();
                    this->state = StateHeadersTrailingSpaceStart;
                    break;
                }
                if(character == ' ') //no spaces allowed between header key and column
                {
                    this->error = 400; //Bad Request
					this->state = StateParsingError;
                    return 1;
                }
                if((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')  
                || (character >= '0' && character <= '9') || (character == '_') || (character == '-')){
                    break;
                }
                std::cout << "this->error: character not allowed" << std::endl;
                this->error = 400; //Bad Request
				this->state = StateParsingError;
                return 1;
            }
            case StateHeadersTrailingSpaceStart: {
                if(character == ' ')
                {
                    break;
                }
                this->content.clear();
                this->state = StateHeaderValue;
                break;
            }
            case StateHeaderValue: {
                
                if(character == ' ')
                {
                    this->headers[to_lower(this->headers_key)] = to_lower(this->content);
                    this->content.clear();
                    this->state = StateHeadersTrailingSpaceEnd;
                    break;
                }
                if(character == '\r')
                {
                    this->headers[to_lower(this->headers_key)] = to_lower(this->content);
                    this->content.clear();
                    this->raw += character;
                    this->state = StateHeaders_CR;
                    continue;
                }
                if(character < 32 || character > 126) //non printable chars
                {
                    this->error = 400; //Bad Request
					this->state = StateParsingError;
                    return 1;
                }
                break;
            
            }
            case StateHeadersTrailingSpaceEnd: {
                if(character == ' '){
                    break;
                }
                if(character == '\r'){
                    this->state = StateHeaders_CR;
                    break;
                }
                this->state = StateHeaderValue;
                break;
            }
            case StateHeaders_CR:{
                if(character == '\n'){
                    this->state = StateHeaders_LF;
                    this->raw += character;
                    continue;
                }
                this->error = 400; //Bad Request
				this->state = StateParsingError;
                return 1;
            }
            case StateHeaders_LF:{
                if(character == '\r')
                {
                    this->state = StateHeadersEnd_CR;
                    this->raw += character;
                    continue;
                }
                this->state = StateHeaderKey;
                break;
            }
            case StateHeadersEnd_CR:{
                if(character != '\n')
                {
                    this->error= 400; //Bad Request
					this->state = StateParsingError;
                    return 1;
                }
                if(!this->headers["content-length"].empty())
                {
                    this->has_body = true;
                    this->is_chunked = false;
                }
                else if(!this->headers["transfer-encoding"].empty())
                {
                    this->has_body = true;
                    this->is_chunked = true;
                }
                if(this->has_body && this->method == "POST"){
                    if(this->is_chunked)
                        this->state = StateBodyChunkedNumber;
                    else
                        this->state = StateBodyPlainText;
                    this->raw += character;
                    this->content.clear();
                    continue;
                }
                this->raw += character;
                this->content.clear();
                this->state = StateParsingComplete;
                continue;
            }
            case StateBodyPlainText:{
                if(this->body_counter < strToInt(this->headers["content-length"]))
                {
                    this->body_counter++;
                    this->body += character;
                    this->raw+=character;
                    continue;
                }
                if(this->body_counter == strToInt(this->headers["content-length"]))
                {
                    //TODO check if the last character should be added to this->raw
                    //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/8
                    this->state = StateParsingComplete;
                    continue;
                }
                break;
            }
            case StateBodyChunkedNumber:{
                if(character == '\r')
                {
                    std::stringstream ss;
                    ss << std::hex << this->content;
                    ss >> this->number;
                    if (!(ss >> this->number))
                    {
                        this->error = 400; // conversione in hex fallita
                        this->state = StateParsingError;
                        return 1;
                    }
                    if(this->number == 0)
                    {
                        this->content.clear();
                        this->state = StateChunkedEnd_LF;
                        continue;
                    }
                    this->content.clear();
                    this->raw += character;
                    this->state = StateChunkedNumber_LF;
                    continue;
                }
                if (!isxdigit(character)) {
                    this->error = 400;  //carattere non esadecimale
                    this->state = StateParsingError;
                    return 1;
                }
                this->content += character;
                break;
            }
            case StateChunkedNumber_CR : {
                if(character != '\r')
                {
                    this->error= -1;
                    return 1;

                }
                this->state = StateChunkedChunk_LF;
                continue;
            }
            case StateChunkedNumber_LF : {
                if(character != '\n'){
                    this->error= -1;
                    return 1;

                }
                this->state = StateChunkedChunk;
                continue;
            }
            case StateChunkedChunk : {  
                if(this->content.size() < this->number)
                {
                    this->content += character;
                    this->raw += character;
                    continue;
                }
                if(character == '\r')
                {
                    this->body += this->content;
                    this->raw += character;
                    this->content.clear();
                    this->state = StateChunkedChunk_LF;
                    continue;
                }
                break;
            }
            case StateChunkedChunk_LF : {  
                if(character != '\n')
                {
                    this->error = -1;
                    return 1;
                    
                }
                this->raw += character;
                this->state = StateBodyChunkedNumber;
                continue;
            }
            case StateChunkedEnd_LF:{
                if(character != '\n')
                {
                    this->error = -1;
                    return 1;
                }
                //TODO understand if i need to handle an additional CRLF at the end of the body, after the 0/r/n
                //Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/7
                this->raw += character;
                this->state = StateParsingComplete;
                break;
            }
        
            case StateParsingComplete:{
                return 0;
            }
            

            default:
                std::cout << "case not handled ==> " << character << std::endl;
        }
        this->raw += character;
        this->content += character;
    }

    return 1;
}

void Request::printHeaders() {
    std::string headers = "";
    for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); ++it) {
        headers += it->first + ": " + it->second + "\n";
        std::cout << it->first << " : " << it->second << std::endl;
    }
}

void Request::print_Request() {
    std::cout << "Is Chunked: " << (this->is_chunked ? "true" : "false") << std::endl;
    std::cout << "State: " << this->state << std::endl;
    std::cout << "Method: " << this->method << std::endl;
    std::cout << "URL: " << this->url << std::endl;
    std::cout << "Version: " << this->version << std::endl;
    std::cout << "Headers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Body: " << this->body << std::endl;
    std::cout << "Error: " << this->error << std::endl;


    std::cout << "--------------------------------" << std::endl;
    std::cout << "Raw: " << this->raw << std::endl;

}

std::string &Request::getUrl()  {return this->url;}

std::string &Request::getVersion()  {return this->version;}

std::map<std::string, std::string> &Request::getHeaders()   {return this->headers;}

bool &Request::hasBody()    {return this->has_body;}

void Request::setHasBody(bool hasBody)  {this->has_body = hasBody;}

void Request::setUrl(std::string& url)  {this->url = url;}

void Request::setVersion(std::string& version)  {this->version = version;}

void Request::setHeaders(std::map<std::string, std::string>& headers)   {this->headers = headers;}

void Request::setBody(std::string& body)    {this->body = body;}


Request::~Request(){};

Request::Request() 
        : 
        state(StateMethod),
        has_body(false),
        is_chunked(false),
        error(0),
        number(0),
        body_counter(0),
        encoded_counter(0)
{
    //servono nel parsing della request
    this->methods[GET] = "GET";
    this->methods[POST] = "POST";
    this->methods[DELETE] = "DELETE";
    
};
