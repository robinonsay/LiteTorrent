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


#define PORT 8000
#define FILESIZE 30

void start_thread(){
	std::cout << "Oh, and here's a thread!" << std::endl;

}

int main(int argc, char * argv[])
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		std::cout << "Error: creating socket failed" << std::endl;
        exit(0);
	}
	// Allow port number to be reused 
	int yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		std::cout << "Error: setting socket option failed" << std::endl;
		exit(0);
	}
	// Bind socket to an address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 0.0.0.0
	addr.sin_port = htons(PORT);
	if (bind(sock, (sockaddr *) &addr, sizeof(addr)) == -1) {
		std::cout << "Error: binding socket failed" << std::endl;
		exit(0);
	}
	if (listen(sock, 10) == -1) 
    { 
        std::cout << "Error: listening socket failed" << std::endl; 
		exit(0);
    } 
    socklen_t addr_len = sizeof(addr);
    int connection = accept(sock, (sockaddr *) &addr, &addr_len);
    if (connection == -1) {
    	std::cout << "Error: accepting connection failed" << std::endl;
		exit(0);
	}

	char data[FILESIZE];
	memset(data, 0, sizeof(data));
	ssize_t recvbyte = recv(connection, data, FILESIZE, MSG_WAITALL);
	if (recvbyte == -1)
	{
		fprintf(stderr, "recv: %s (%d)\n", strerror(errno), errno);
	}
	std::ofstream out_file;
	out_file.open("new-file.txt", std::ios::binary);
	out_file.write(data, recvbyte);
	out_file.close();

	std::cout << "Successfully Received File" << std::endl;

	std::thread t(start_thread);
	t.join();


	return 0;
}