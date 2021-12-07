#include <iostream>
#include <cstring>
#include <string>
#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

// Server listen this port for client connections
const short SERVER_PORT = 10000;

const int SEND_BLOCK_SIZE = 1024;

static void print_usage()
{
    cout << "   send_file [OPTIONS] [IP] [FILE]" << endl;
    cout << "   -s - server mode. File receiver. Listen incoming connection from clients and " << endl;
    cout << "   	receive file" << endl;
    cout << "   -c - client mode. Open connection to server, send file, and close." << endl;
    cout << "   " << endl;
    cout << "   IP - server IP(client only)" << endl;
    cout << "   FILE - sending file(client only)" << endl;
}

static int client(int argc, char *argv[])
{
    char *buffer;
    int r;
    
    // Parse parameters
    if(argc != 4)
    {
	cerr << "IP and FILE must be specified for client mode" << endl;
	print_usage();
	return -1;
    }

    const char *ip = argv[2];
    const char *filename = argv[3];

    // Open file
    fstream file;

    file.open(filename, ios::binary | ios::in);

    if(!file.is_open())
    {
	cerr << "Open file error" << endl;
	return -1;
    }

    // Open connection to the server
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
	cerr << "Create socket error" << endl;
	goto error;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(ip);

    r = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if(r < 0)
    {
	cerr << "Connect to server error" << endl;
	goto error;
    }

    cout << "Connect to server success" << endl;

    // Read the file block by block and send it server
    buffer = new char[SEND_BLOCK_SIZE];

    while(file.eof())
    {
	int readed = file.readsome(buffer, SEND_BLOCK_SIZE);
	if(file.fail())
	{
	    cerr << "Read file error" << endl;
	    goto error;
	}

	r = send(sock, buffer, readed, 0);
	if(r == -1)
	{
	    cerr << "File transfer error." << endl;
	    goto error;
	}
    }

    cout << "File transfere complete" << endl;
	
error:
    file.close();
    close(sock);
    delete buffer;

    return 0;
}

/* Send file to the specified IP address. In the received PC 
   this programm must be running in server mode. It receive file and 
   save with name [data]_[time].hex
   Calling options:
   
   send_file [OPTIONS] [IP] [FILE]

   -s - server mode. File receiver. Listen incoming connection from clients and 
   	receive file
   -c - client mode. Open connection to server, send file, and close.
   
   IP - server IP(client only)
   FILE - sending file(client only)
*/
 
int main(int argc, char *argv[])
{
    // Parameters
    enum {CLIENT, SERVER} mode;

    //
    // Parse parameters.
    //

    if(argc < 2)
    {
	cerr << "Invalid parameters: mode is not specified" << endl;
	print_usage();
	return -1;
    }
    
    // Mode
    const char *mode_str = argv[1];

    if( strcmp(mode_str, "-s") )
    {
	mode = SERVER;
    }
    else if( strcmp(mode_str, "-c"))
    {
	mode = CLIENT;
    }
    else
    {
	cerr << "Invalid parameters: mode is not specified" << endl;
	print_usage();
	return -1;
    }

    if( mode == CLIENT )
    {
	return client(argc, argv);
    }
    else
    {
	/* Server mode */
	
    }
	
    return 0;
}
