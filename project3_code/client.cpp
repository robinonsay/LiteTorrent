#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <chrono>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <netdb.h>
#include <thread>
#include <pthread.h>
#include <iostream>
#include <fstream>

#define SERVER "localhost"
#define FILESIZE 30
#define PORT 8000


void start_thread()
{
	std::cout <<"Oh, and here's a thread!" << std::endl;
	
}


int main(int argc, char * argv[])
{

	// Create socket 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		std::cout << "Error: creating socket failed" << std::endl;
        exit(0);
	}
	// connect to the server's binded socket 
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 0.0.0.0
	addr.sin_port = htons((u_short) PORT);
	struct hostent* name = gethostbyname(SERVER);
	memcpy(&addr.sin_addr, name->h_addr, name->h_length);
	

	if(connect(sock, (sockaddr*) &addr, sizeof(addr)) == -1)
	{
		std::cout << "Error: can't connect to server" << std::endl;
        exit(0);
	}


	char data[FILESIZE];
	memset(data, 0, sizeof(data));

	std::ifstream in_file;
	in_file.open("example-file.txt", std::ios::binary);
	in_file.read(data, FILESIZE);
	unsigned int dataRead = in_file.gcount();

	ssize_t sendbyte = send(sock, data, dataRead, MSG_NOSIGNAL);
	if (sendbyte == -1) {
    	std::cout << "Error: sending data failed" << std::endl;
		exit(0);
	}
	in_file.close();

	std::cout << "Succesfully Sent File" << std::endl;

	std::thread t(start_thread);
	t.join();

	return 0;
}