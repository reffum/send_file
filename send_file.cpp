#include <iostream>
#include <string>

using namespace std;


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

    
    
    return 0;
}
