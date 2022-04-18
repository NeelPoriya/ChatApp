%.o: %.c
	gcc -c $<

server: server.o client.o client
	gcc -pthread  -o server server.o

client: client.o
	gcc -pthread -o client client.o
