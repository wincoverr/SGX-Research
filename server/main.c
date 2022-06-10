//include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

//include for socket
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 80
#define PORT 8083
#define SA struct sockaddr


int decodeBuffer( char buff[MAX]){
   
    if(strcmp(buff, "-help\n") == 0){
        return 999999;
    }else if(strcmp(buff, "-login\n") == 0){
        return 999998;
     }else if(strcmp(buff, "-exit\n") == 0){
        return 999997;
    }else
        return 1;
}

int RequestManager(char buff[MAX])
{
    int decoded = decodeBuffer(buff);
    switch (decoded)
    {
        case 999999:
        printf("[REQUEST-MANAGER] Help Request \n");
        return 999999;
        break;

        case 999998:
        printf("[REQUEST-MANAGER] Client want to connect \n");
        return 999998;
        break;

        case 999997:
        printf("[REQUEST-MANAGER] Client leaves \n");
        return 999997;
        break;

        default:
        printf("[REQUEST-MANAGER] Not a request message \n");
        return 0;
        break;
    }
}



void func(int connfd)
{
    char buff[MAX];
    int n;
    int response;
    // infinite loop for chat
    for (;;) {
        
        bzero(buff, MAX);
        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));

        // print buffer which contains the client contents
        printf("From client: %s", buff);

        RequestManager function all here
       // response = RequestManager(buff);
        // and send that buffer to client
        strcpy(buff, "exit");
        
        write(connfd, buff, sizeof(buff));
   
        //server never exit
        
        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
        
    }
}

/*  
    
    Todo:       
    1 - create a server that is waiting connexion of a client
    2 - send data to client
    3 - can receive data from client and print it on a txt file                
                            */

int main(int argc, char *argv[])
{
 
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
   for(;;){
        // Now server is ready to listen and verification
        if ((listen(sockfd, 5)) != 0) {
            printf("Listen failed...\n");
            exit(0);
        }
        else
            printf("Server listening..\n");
        len = sizeof(cli);
    
        // Accept the data packet from client and verification 
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");
    
        // Function for chatting between client and server
        func(connfd);
        printf("SERVER DISCONNECTED");
        //After chatting close the socket
        close(sockfd);
    }
}