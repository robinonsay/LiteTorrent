all: client server
client: client.cpp
	g++ -std=c++11 -g -Wall -o client client.cpp -pthread
server: server.cpp
	g++ -std=c++11 -g -Wall -o server server.cpp -pthread
clean:
	$(RM) client
	$(RM) server