CC = g++

CFLAGS = -c -Wall

all: echo

echo: client server

client: client.o
	$(CC) echo_client/client.o -o client -lsfml-system -lsfml-network -lpthread

client.o: echo_client/main.cpp
	$(CC) $(CFLAGS) echo_client/main.cpp -o echo_client/client.o

server: server.o
	$(CC) echo_server/server.o -o server -lsfml-system -lsfml-network -lpthread

server.o: echo_server/main.cpp
	$(CC) $(CFLAGS) echo_server/main.cpp -o echo_server/server.o

clean:
	rm echo_client/*.o echo_server/*.o client server
