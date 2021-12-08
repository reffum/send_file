#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

using namespace std;

// Server listen this port for client connections
const short SERVER_PORT = 10000;

const int SEND_BLOCK_SIZE = 1024;

/* CTRL+C handler */
void sigingHandler(int signum)
{
	cout << "CTRL-C hapenned. Program exit" << endl;
	exit(0);
}

/*
 * Return file name for file in format:
 * [data]_[time].hex
 * Sample: 26.10.2021_12.02.54.hex
 */
static string getFileName()
{
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    stringstream str;

    str << timeinfo->tm_wday << "." 
	<< timeinfo->tm_mon  << "."
	<< timeinfo->tm_year << "_"
	<< timeinfo->tm_hour << "."
	<< timeinfo->tm_min  << "."
	<< timeinfo->tm_sec;

    return str.str();
}

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

    file.close();
    close(sock);
    delete buffer;
    return 0;
	
error:
    file.close();
    close(sock);
    delete buffer;

    return -1;
}

static int server(int argc, char *argv[])
{
    int listen_sock, sock;
    struct sockaddr_in addr;
    char *buffer;
    int r;

    // Open listen connection on SERVER_PORT for 1 client
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sock == -1)
    {
	cerr << "Create socket error" << endl;
	return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    r = bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr));
    if(r < 0)
    {
	cerr << "Bind error" << endl;
	goto error;
    }

    while(1)
    {
	r = listen(listen_sock, 1);
	if(r < 0)
	{
	    cerr << "Listen error" << endl;
	    goto error;
	}

	sock = accept(listen_sock, NULL, NULL);
	if(sock < 0)
	{
	    cerr << "Accept error" << endl;
	    continue;
	}

	// Create new file
	string filename = getFileName();

	fstream file;
	file.open(filename, ios::binary | ios::out);

	if(!file.is_open())
	{
	    cerr << "Open file error" << endl;
	    goto error;
	}

	buffer = new char[SEND_BLOCK_SIZE];

	int readed = 0;
	do
	{
	    readed = recv(sock, buffer, SEND_BLOCK_SIZE, 0);

	    if(readed < 0)
	    {
		cerr << "receive file error" << endl;
		goto error;
	    }

	    file.write(buffer, readed);
	} while(readed > 0);

	/* Connection closed */
	cout << "File " << filename << "received" << endl;
	file.close();
    }

error:
    close(listen_sock);
    close(sock);
    return -1;
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
    signal(SIGINT, sigingHandler);
    
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

    if( !strcmp(mode_str, "-s") )
    {
	mode = SERVER;
    }
    else if( !strcmp(mode_str, "-c"))
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
	return server(argc, argv);
    }
	
    return 0;
}
