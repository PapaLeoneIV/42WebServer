#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#include "../includes/ConfigParser.hpp"

int handle_arguments(int argc, char **argv)
{
    // help command
        if (argc == 2)
        {
            if (std::string(argv[1]) == "--help"){
                Logger::info("Usage: webserver { [OPTIONS] || <config-filepath> }");
                Logger::info("         --help                           Display this help and exit");
                Logger::info("         -t <config-filepath>             Test the configuration file");
                Logger::info("         -v                               Display the current version");
                return(1);
            }
            // version command
            if (std::string(argv[1]) == "-v"){
                Logger::info("webserver version: webserver/1.0.0");
                return(1);
            }
            // testing config file command
            if (std::string(argv[1]) == "-t")
            {
                Logger::info("webserver config-file testing: missing <config-filepath>");
                return(1);
            }
            
            return 0;
            
        } else if(argc == 3) {
            if (std::string(argv[1]) != "-t"){
                Logger::info("webserver: try 'webserver --help' for more information");
                return(1);
            }
            ConfigParser configParser;
            if (configParser.validatePath(argv[2]) || configParser.fromConfigFileToServers(argv[2])){
                Logger::error(configParser.getFileName(), "config-file testing: KO");
                return(1);
            }
            Logger::info("webserver config-file testing: OK");
            return(1);
        } else {
            Logger::info("webserver: try 'webserver --help' for more information");
            return(1);
        }
        return 0;
} 

std::string fromDIRtoHTML(std::string dirPath, std::string url)
{
    (void)url;
    std::string html = "<!DOCTYPE html>"
                       "<html lang=\"en\">"
                       "<head>"
                       "<meta charset=\"UTF-8\">"
                       "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                       "<title>WebServer</title>"
                       "</head>"
                       "<body>";
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dirPath.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (url != "/")
            {
                html += "<li><a href=\"" + url + std::string(ent->d_name) + "\">" + std::string(ent->d_name) + "</a></li>";
            }
            else
            {
                html += "<li><a href=\"" + std::string(ent->d_name) + "\">" + std::string(ent->d_name) + "</a></li>";
            }
        }
        closedir(dir);
    }
    else
    {
        return "Error: could not open directory";
    }
    html += "</ul></body></html>";
    return html;
}

std::string readTextFile(std::string filePath)
{
    std::string fileContent;
    std::ifstream file(filePath.c_str(), std::ios::in);
    std::string line;
    while (std::getline(file, line))
    {
        fileContent += line + "\n";
    }
    file.close();
    return fileContent;
}

std::string readFileBinary(std::string filePath)
{
    std::string fileContent;
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    for (std::vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it)
    {
        fileContent.push_back(*it);
    }

    return fileContent;
}

std::string ErrToStr(int error)
{
    switch (error)
    {
    case SUCCESS:
        return "Success";
    case ERR_RESOLVE_ADDR:
        return "Could not resolve address";
    case ERR_SOCK_CREATION:
        return "Error: socket creation failed";
    case ERR_SOCKET_NBLOCK:
        return "Error: setting socket to non-blocking failed";
    case ERR_BIND:
        return "Error: bind failed";
    case ERR_LISTEN:
        return "Error: listen failed";
    case ERR_SELECT:
        return "Error: select failed";
    case ERR_SEND:
        return "Error: send failed";
    case ERR_RECV:
        return "Error: recv failed: closing connection";
    case INVALID_METHOD:
        return "Error: the method is not supported (yet)";
    case INVALID_URL:
        return "Error: Invalid URL";
    case INVALID_VERSION:
        return "Error: HTTP version not supported";
    case INVALID_MANDATORY_HEADER:
        return "Error: Missing mandatory header";
    case INVALID_BODY:
        return "Error: Invalid body";
    case INVALID_BODY_LENGTH:
        return "Error: Invalid body length";
    case INVALID_MAX_REQUEST_SIZE:
        return "Error: Request too long";
    case INVALID_CONNECTION_CLOSE_BY_CLIENT:
        return "Error: Connection closed by client";
    case INVALID_REQUEST:
        return "Error: Invalid request";
    case INVALID_CONTENT_LENGTH:
        return "Error: Invalid content length";
    case ERR_FCNTL:
        return "Error: fcntl failed";
    case FILE_NOT_FOUND:
        return "Error: File not found";
    case FILE_READ_DENIED:
        return "Error: Read access denied";
    default:
        return "Unknown Error";
    }
}
static char hexToAsciiChar(const std::string &hex)
{
    if (hex.length() != 2)
    {
        throw std::invalid_argument("Hex string must be exactly 2 characters long.");
    }

    int decimalValue;
    std::istringstream(hex) >> std::hex >> decimalValue;
    return static_cast<char>(decimalValue);
}

std::string getContentType(std::string &url, int status)
{
    if (status != 200)
        return "text/html";
    if (url == "/")
        return "text/html";
    if (*(url.rbegin()) == '/')
        return "text/html";
    size_t idx = url.find_last_of(".");
    if (idx == std::string::npos)
        return "text/plain";
    std::string extension = url.substr(idx);
    if (extension.empty())
        return "text/plain";
    std::string urlC = &extension[0];
    if (urlC == ".css")
    {
        return "text/css";
    }
    if (urlC == ".csv")
    {
        return "text/csv";
    }
    if (urlC == ".gif")
    {
        return "image/gif";
    }
    if (urlC == ".htm")
    {
        return "text/html";
    }
    if (urlC == ".html")
    {
        return "text/html";
    }
    if (urlC == ".ico")
    {
        return "image/x-icon";
    }
    if (urlC == ".jpeg")
    {
        return "image/jpeg";
    }
    if (urlC == ".jpg")
    {
        return "image/jpeg";
    }
    if (urlC == ".js")
    {
        return "application/javascript";
    }
    if (urlC == ".json")
    {
        return "application/json";
    }
    if (urlC == ".png")
    {
        return "image/png";
    }
    if (urlC == ".pdf")
    {
        return "application/pdf";
    }
    if (urlC == ".svg")
    {
        return "image/svg+xml";
    }
    if (urlC == ".txt")
    {
        return "text/plain";
    }
    // TODO: add support for error 415 unsupported media type
    return "text/plain";
}

std::string getMessageFromStatusCode(int status)
{
    switch (status)
    {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 411:
        return "Length Required";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 505:
        return "HTTP Version Not Supported";
    default:
        return "Status Code not recognized";
    }
    return "Status Code not recognized";
}

std::string sanitizeDots(std::string string)
{
    size_t pos;
    while ((pos = string.find("..")) != std::string::npos)
    {
        string.erase(pos, 2);
    }
    while ((pos = string.find("../")) != std::string::npos)
    {
        string.erase(pos, 3);
    }
    return string;
}

std::string removeHexChars(std::string &url)
{
    std::string result;
    for (std::size_t i = 0; i < url.length(); ++i)
    {
        if (url[i] == '%' && i + 2 < url.length())
        {
            std::string hex = url.substr(i + 1, 2);
            result += hexToAsciiChar(hex);
            i += 2;
        }
        else
        {
            result += url[i];
        }
    }
    result = sanitizeDots(result);
    return result;
}

ERROR checkPermissions(std::string fullPath, int mode)
{
    ERROR error = 0;
    if ((error = access(fullPath.c_str(), mode)))
    {
        return error;
    }
    return SUCCESS;
}

int strToInt(std::string str)
{
    std::stringstream ss(str);
    int number;
    ss >> number;
    return number;
}

int strToHex(const std::string &str)
{
    int number = 0;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> number;
    return number;
}

std::string intToStr(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string to_lower(const std::string &input)
{
    std::string result = input;
    for (std::string::size_type i = 0; i < result.size(); ++i)
    {
        result[i] = std::tolower(static_cast<unsigned char>(result[i]));
    }
    return result;
}

std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

std::string readBinaryStream(std::istringstream &stream, int size)
{
    int i = 0;
    std::string fileContent;
    while (i < size)
    {
        char ch;
        stream.get(ch);
        fileContent += ch;
        i++;
    }
    return fileContent;
}
// TODO questi venivano usati nel vecchio parsing della request, non buttare via perche alcuni pezzi di codice
// Issue URL: https://github.com/PapaLeoneIV/42WebServer/issues/17
// sono riutilizzabili

// std::string extractBodyFromStream(std::istringstream &iss, const std::string &boundary) {
//     std::string line;
//     std::string content;
//     bool withinBoundary = false;

//     while (std::getline(iss, line)) {
//         if (line == "--" + boundary + "\r") {
//             withinBoundary = true;
//             continue;
//         }
//         if (withinBoundary && line == "--" + boundary + "--\r") {
//             withinBoundary = false;
//             break;
//         }
//         if (withinBoundary) {
//             content += line + "\n";
//         }
//     }
//     return content;
// }
// std::vector<std::string> splitIntoSections(std::istringstream &iss) {
//     std::vector<std::string> sections;
//     std::string line;
//     std::string currentSection;

//     while (std::getline(iss, line, '\n')) {
//         if (line.find("Content-Disposition: form-data;") != std::string::npos) {
//             if (!currentSection.empty()) {
//                 sections.push_back(currentSection);
//                 currentSection = "";
//             }
//         }
//         currentSection += line + "\n";
//     }
//     if (!currentSection.empty()) {
//         sections.push_back(currentSection);
//     }

//     return sections;
// }
// std::map<std::string, std::string> extractSection(const std::string &section) {
//     std::map<std::string, std::string> extractedData;
//     std::istringstream sectionStream(section);
//     std::string line;
//     bool isBody = false;
//     std::string body;

//     while (std::getline(sectionStream, line)) {
//         if (isBody) {
//             body += line + "\n";
//         } else if (line.find("Content-Disposition:") != std::string::npos) {
//             std::string::size_type namePos = line.find("name=\"");

//             if (namePos != std::string::npos) {
//                 namePos += 6;
//                 std::string::size_type endPos = line.find("\"", namePos);
//                 if (endPos != std::string::npos) {
//                     extractedData["name"] = line.substr(namePos, endPos - namePos);
//                 }
//             }

//             std::string::size_type filenamePos = line.find("filename=\"");

//             if (filenamePos != std::string::npos) {
//                 filenamePos += 10;
//                 std::string::size_type endPos = line.find("\"", filenamePos);
//                 if (endPos != std::string::npos) {
//                     extractedData["filename"] = line.substr(filenamePos, endPos - filenamePos);
//                 }
//             }
//         } else if (line.find("Content-Type:") != std::string::npos) {
//             extractedData["contentType"] = line.substr(14);
//         } else if (line == "\r") {
//             isBody = true;
//         }
//     }

//     extractedData["body"] = body;

//     return extractedData;
// }
