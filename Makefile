#Makefile


CC = g++
CFLAGS = -Wall -g

make:
	gcc server.c -o server
	gcc client.c -o client

clean:
	rm -f server client
