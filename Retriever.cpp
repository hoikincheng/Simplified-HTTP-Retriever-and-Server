/**-----------------------------Retriever.cpp--------------------------------
*  Name: Hoikin Cheng.
*  Creation Date: 10/16/2019.
*  Date of Last Modification: 10/25/2019.
*  --------------------------------------------------------------------------
*  Purpose: Retriever.cpp creates a new socket, connects to the server and 
*  sends HTTP request to a server and waits for its response. Then, it 
*  displays the response on the console and save the contents to a file.
*  --------------------------------------------------------------------------
*  
*/
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <strings.h>      // bzero
#include <iostream>       // cout, cerr, endl
#include <stdlib.h>       // atoi
#include <fstream>        // ofstream
#include <sstream>        // stringstream

#include <fcntl.h>        
#include <unistd.h>
#include <string.h>

using namespace std;

const string HTTP_VERSION = "HTTP/1.0";
const int BUFFER_SIZE = 1024;
const string OUTPUT = "Output.txt";
const string GET_METHOD = "GET";

// Not supported by Server; for testing purposes
const string POST_METHOD = "POST";

const string END_LINE = "\r\n";
const string HEADER_END = "\r\n\r\n";
const string HTTP = "http://";
const string HTTPS = "https://";


/**--------------------------------main----------------------------------
*  Description: The main method receives inputs from command-line and 
*  makes a connection to the server, retrieve message from server, and 
*  outputs the contents to a file.
*  ----------------------------------------------------------------------
*  @param [server's IP address] [path] [port number].
*  @return int -1 if there are errors. Otherwise 0.
*  ----------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cerr << "Wrong arguments format." << endl;
        cerr << "Please follow the format: server address, path, " << 
        "port number" << endl;
        return -1;
    }
    
    const char *serverAddress = argv[1] + '\0';
    
    // Remove htt:// or https:// from the server's address
    int rt1 = string(serverAddress).find(HTTP);
    int rt2 = string(serverAddress).find(HTTPS);
    string tempStr;
    
    if (rt1 == 0)
    {
        tempStr = string(serverAddress).substr(rt1 + 7);
    }
    else if (rt2 == 0)
    {
        tempStr = string(serverAddress).substr(rt2 + 8);
    }
    else
    {
        tempStr = string(serverAddress);
    }
    const char * serverAddr = tempStr.c_str();
    
    int port = atoi(argv[3]);
    if ( port > 65535 )
    {
        cerr << "Invalid port number. Port number must be less than " << 
        "65535." << endl;
        return -1;
    }

    
    // Retrieve a hostent structure corresponding to this IP name
    struct hostent * host = gethostbyname(serverAddr);
    if (host == NULL)
    {
        cerr << "Error retrieving hostent." << endl;
        return -1;
    }
    
    
    // Declare a sockaddr_in structure, zero-initialize it by calling bzero, 
    // and set its data members
    sockaddr_in sendSockAddr;
    bzero( (char*)&sendSockAddr, sizeof( sendSockAddr ) );
    sendSockAddr.sin_family      = AF_INET; // Address Family Internet
    sendSockAddr.sin_addr.s_addr =
      inet_addr( inet_ntoa( *(struct in_addr*)*host->h_addr_list ) );
    sendSockAddr.sin_port        = htons( port );
    
    // Open a stream-oriented socket with the Internet address family
    int clientSd = socket( AF_INET, SOCK_STREAM, 0 );
    if (clientSd < 0)
    {
        cerr << "Error: failed to open socket." << endl;
        close(clientSd);
        return -1;
    }


    //Connect this socket to the server by calling connect
    int connectStatus = connect( clientSd, ( sockaddr* )&sendSockAddr, 
      sizeof( sendSockAddr ) );
    if (connectStatus < 0)
    {
        cerr << "Error: failed to connect to the server." << endl;
        close(clientSd);
        return -1;
    }
    
    char buf[BUFFER_SIZE];
    
	string requestMessage;
	
    // Flag for GET or POST method in HTTP Request
    // Positive number -> GET method
    // Non-positive number -> POST method
    int methodOption = 1;
    
    if (argc >= 5)
    {
        methodOption = atoi(argv[4]);
    }
    
    // Changes method in the request message header depending on if it is a
    // Positive number -> GET method,
    // Non-positive number -> POST method.
    if (methodOption > 0)
    {
        requestMessage = GET_METHOD + " ";
    }
    else
    {
        requestMessage = POST_METHOD + " ";
    }
    
    
	requestMessage += string(argv[2]);
	requestMessage += " " + HTTP_VERSION + END_LINE + 
	"Host: ";
	requestMessage += string(serverAddr) + HEADER_END;
	
	/*
	cout << "HTTP Request Message: " << endl;
	cout << requestMessage << endl << endl;
	*/
	
	// Send HTTP Request message to server
	int writeRt = write(clientSd, requestMessage.c_str(), requestMessage.length());
	if (writeRt < 0)
	{
        cerr << "Did not write: " << writeRt << endl;
	}
    
    string responseMessage;
    int readRt; // The size of bytes read
	
	// Read in HTTP Response message from server
	do 
	{
        readRt = read(clientSd, buf, BUFFER_SIZE);
        buf[readRt] = '\0';
	    responseMessage += buf;
    } while (readRt > 0);
    
    stringstream responseMessageStream(responseMessage);
    string version;
    int statusCode;
    string statusPhrase;
    
    responseMessageStream >> version >> statusCode >> statusPhrase;


    // Find where the header ends and where the message body begins
	int headerEndPosition = responseMessage.find(HEADER_END);
	string responseBody = responseMessage.substr(headerEndPosition + 4);
	
    // Display the contents of retrieved file from server
    // If there is an error, this will display the error page contents
    cout << responseBody << endl;
    
	// Only writes to Output.txt when the status code is 200 (OK)
    if (statusCode == 200)
    {
	    ofstream file (OUTPUT);
	    if (file)
	    {
		    file << responseBody;
	    }
	}
	
	close(clientSd);
	
    return 0;
}
