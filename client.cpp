#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <string.h>
#include <vector>
#include <algorithm>

using namespace std;

int main(int argc, char* argv[]) {

    //./client 10001 “add_voter 9999” `
	if(argc !=3){
        std::cout << "Usage: ./client <port> \"<<command_name> <arg1> <arg2> … <argN>>\"" << std::endl;
        return -1;
    }
    int portnumber = std::stoi(argv[1]);
    if(portnumber < 10000 || portnumber > 65535){
        std::cout << "Invalid port, choose 10000-65535 inclusive" << std::endl;
        return -1;
    }
    std::string argument = argv[2];
    argument.erase(std::remove(argument.begin(), argument.end(), '\"'), argument.end());

    //set the socket
    struct sockaddr_in st;
    st.sin_family = AF_INET; // use IPv4
    st.sin_addr.s_addr = INADDR_ANY; // bind to the current IP of this machine
    st.sin_port = htons(portnumber); // use this port

    // create a stream socket on top of IPv4; can use 0 for the last argument: UDP is the default for datagram sockets
    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sd == -1) {
        cout << "ERROR: create socket"  << endl;
        shutdown(sd, SHUT_RDWR);
        return -1;
    }

	//connect to server
	if (connect(sd, (const struct sockaddr*)&st, sizeof(st)) == -1)  {
      std::cout << "ERROR: connection to server"  << std::endl;
      shutdown(sd, SHUT_RDWR);
      return -1;
    }

	const int MAXNUM_CHAR = 1024;
	//send to server
    int length = argument.length();
    char csend[length+1];
    strcpy(csend, argument.c_str());
    int byte_send = send(sd, csend, length+1, 0);
    if( byte_send < length+1) {
        std::cout << "ERROR: sening message" << std::endl;
        shutdown(sd, SHUT_RDWR);
        return -1;
    }

	//get the result from server
    char creceive[MAXNUM_CHAR];
    int bytes_received = recv(sd,creceive,MAXNUM_CHAR,0);
    if(bytes_received <= 0) {
        std::cout << "ERROR: receiving message" << std::endl;
        shutdown(sd, SHUT_RDWR);
        return -1;
    }
    creceive[bytes_received] = '\0';
    std::string rcv;
    rcv = creceive;

    std::cout << rcv << std::endl;
	shutdown(sd, SHUT_RDWR);
    return 0;
}
