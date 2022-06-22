all: server client

server: server.c user.c
	gcc -pthread  -o server server.c

client: client.c user.c
	gcc -pthread -o client client.c
