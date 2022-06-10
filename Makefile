#Makefile


CC = g++
CFLAGS = -Wall -g


make:
	cd client && $(CC) main.c -o client
	cd server && $(CC)  main.c -o server

source:
	cd .. && source environment 
	
server:
	cd server && gcc main.c -o server
	cd server && ./server

client:
	cd client && gcc main.c -o client
	cd client && ./client

clean:
	cd client && rm -rf client
	cd server && rm -rf server

runclient:
	cd client && ./client

runserver:
	cd server && ./server