/**-------------------------------Server.cpp---------------------------------
*  Name: Hoikin Cheng.
*  Creation Date: 10/16/2019.
*  Date of Last Modification: 10/25/2019.
*  --------------------------------------------------------------------------
*  Purpose: Server.cpp creates a TCP socket that listens on a port. The 
*  server will accept an incoming connection and then create a new thread 
*  that will handle the connection. The Server will retrieve the file 
*  specified by the HTTP request message and sends its contents back.
*  --------------------------------------------------------------------------
*  
*/
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <unistd.h>       // read, write, close
#include <strings.h>      // bzero
#include <iostream>       // cout, cerr, endl
#include <sstream>        // stringstream
#include <fstream>        // ifstream getline

// remove unnecessary information output to console

using namespace std;

const int BUFFER_SIZE = 1024;
const int QUEUE_MAX_LENGTH = 10;

const string HTTP_VERSION = "HTTP/1.0";
const string OK = "200 OK";
const string BAD_REQUEST = "400 Bad Request";
const string UNAUTHORIZED = "401 Unauthorized";
const string FORBIDDEN = "403 Forbidden";
const string NOT_FOUND = "404 Not Found";

const string BAD_REQUEST_PATH = "/BadRequest.html";
const string UNAUTHORIZED_PATH = "/Unauthorized.html";
const string FORBIDDEN_PATH = "/Forbidden.html";
const string NOT_FOUND_PATH = "/NotFound.html";

const string SECRET_FILE = "SecretFile.html";
const string END_LINE = "\r\n";
const string HEADER_END = "\r\n\r\n";

int port;
int serverSd;

/**----------------------------threadServer------------------------------
*  Description: The threadServer method reads data from a client's HTTP 
*  request message, retrieves the file request, and sends the file's 
*  contents back. If file cannot be retrieved for any reason, then an 
*  error page is sent back.
*  ----------------------------------------------------------------------
*  @param threadData The thread_data type that stores the client socket
*  file descriptor.
*  @return Nothing.
*  ----------------------------------------------------------------------
*/
void *threadServer(void *threadData)
{
    int clientSd = *(int*)threadData;
    char buf[BUFFER_SIZE];
    string requestMessage;
    stringstream requestMessageStream;
    string method; // Method of HTTP Request Message
    string path; // Path (URL) of HTTP Request Message
    int readRt; // The size of bytes read
    string responseMessage;
    
    // Reads in the HTTP Request Message from client
    do 
    {
        readRt = read(clientSd, buf, BUFFER_SIZE);
        buf[readRt] = '\0';
	    requestMessage += buf;
	    if (requestMessage.find(HEADER_END))
	    {
	        break;
	    }
    } while (readRt > 0);
    
    //cout << "REQUEST MESSAGE:" << endl;
    //cout << requestMessage << endl;
    
	requestMessageStream.str(requestMessage);
	
	// Extract method and path from the HTTP header message
	requestMessageStream >> method >> path;
	
	
	if (method == "GET") // Only the GET method is supported
	{
	    if (path.substr(0, 2) == "..")
        {
            responseMessage = HTTP_VERSION + " " + FORBIDDEN + HEADER_END;
            path = FORBIDDEN_PATH;
        }
        else if (path.find(SECRET_FILE) != -1)
        {
            responseMessage = HTTP_VERSION + " " + UNAUTHORIZED + HEADER_END;
            path = UNAUTHORIZED_PATH;
        }
        else
        {
            responseMessage = HTTP_VERSION + " " + OK + HEADER_END;
        }
	}
	else // methods other than GET is not supported
	{
	    responseMessage = HTTP_VERSION + " " + BAD_REQUEST + HEADER_END;
	    path = BAD_REQUEST_PATH;
	}
	
    path = "." + path;
    FILE *file = fopen(path.c_str(), "r");
    
    // If file cannot be opened, change path to file not found html
    if (file == nullptr)
    {
        responseMessage = HTTP_VERSION + " " + NOT_FOUND + HEADER_END;
        path = "." + NOT_FOUND_PATH;
        file = fopen(path.c_str(), "r");
    }
    while (!feof(file))
    {
        char c = fgetc(file);
        // Move on if fstream encounters character that are not supported
        if ( c < 0 )
        {
            continue;
        }
        responseMessage += c;
    }
    
    // Closes file after writing its content to HTTP response message's body
    fclose(file);
    
    //cout << "RESPONSE MESSAGE:" << endl;
    //cout << responseMessage << endl;
    
    // Send HTTP Response message to client
	int writeRt = write(clientSd, responseMessage.c_str(), responseMessage.length());
	if (writeRt < 0)
	{
        cerr << "Did not write: " << writeRt << endl;
	}

	close(clientSd);
	pthread_exit(NULL);
    return 0;    
}

/**--------------------------------main----------------------------------
*  Description: The main method receives inputs from command-line and 
*  listens for Client's connections. It creates a thread to handle 
*  client's request after the HTTP request is received.
*  ----------------------------------------------------------------------
*  @param [port].
*  @return int -1 if there are errors. Otherwise 0.
*  ----------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Wrong arguments format." << endl;
        cerr << "Please follow the format: port#." << 
        endl;
        return -1;
    }
    
    port = atoi(argv[1]);
    if ( port < 1024 || port > 65535 )
    {
        cerr << "Invalid port number. Port number must be between " << 
        "1024 and 65535." << endl;
        return -1;
    }
    
    // Declare a sockaddr_in structure, zero-initialize it by calling bzero, 
    // and set its data members
    sockaddr_in acceptSockAddr;
    bzero( (char*)&acceptSockAddr, sizeof( acceptSockAddr ) );
    acceptSockAddr.sin_family      = AF_INET; // Address Family Internet
    acceptSockAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    acceptSockAddr.sin_port        = htons( port );
    
    
    // Open a stream-oriented socket with the Internet address family
    serverSd = socket( AF_INET, SOCK_STREAM, 0 );
    
    // Set the SO_REUSEADDR option
    const int on = 1;
    setsockopt( serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, 
                sizeof( int ) );
    
    // Bind this socket to its local address            
    bind( serverSd, ( sockaddr* )&acceptSockAddr, sizeof( acceptSockAddr ) );
    
    // Instruct the operating system to listen to up to QUEUE_MAX_LENGTH 
    // connection requests from clients at a time
    listen(serverSd, QUEUE_MAX_LENGTH);
    
    // Receive a request from a client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    
    while(true)
    {
        int newSd = accept(serverSd, (sockaddr *)&newSockAddr, 
          &newSockAddrSize);
    
        // Create a new thread
        pthread_t newThread;
    
        int iret1 = pthread_create (&newThread, NULL, threadServer, 
          (void *)&newSd);
          
        pthread_join(newThread, NULL);
    }
}
