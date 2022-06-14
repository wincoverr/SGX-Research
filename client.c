#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "functions.h"

void responceManager(char buff[MAX], char message[MAX], char username[MAX], char password[MAX], int *msg_type)
{
    int response;
    printf("\n-----------------------------------------------------\n");
    response = atoi(buff);
    // printf("[RESPONCE-MANAGER] Response: %d \n", response);
    switch (response)
    {
    case 999999:
        printf("help commands: \n \n \n \n \n \n \n \n");
        printf("Message : ");
        *msg_type = 4;
        break;
    case 999998:
        printf("USERNAME : ");
        scanf("%s", username);
        printf("PASSWORD : ");
        scanf("%s", password);
        *msg_type = 2;
        break;
    case 999997: // client leaves
        printf("Client Leaves \n");
        *msg_type = 3;
        break;
    default:
        printf("Message : ");
        *msg_type = 1;
        break;
    }
}

int main(int argc, char **argv)
{

    struct message message;
    char intToChar[MAX];
    char *ip = "127.0.0.1";
    int sockfd, n;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buff[MAX];

    strcpy(message.message, "\0");
    strcpy(message.username, "\0");
    strcpy(message.password, "\0");
    message.message_type = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ip);
    strcpy(buff, "");
    responceManager(buff, message.message, message.username, message.password, &message.message_type);

    for (;;)
    {
        bzero(buff, MAX);
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;

        strcpy(message.message, buff);

        //printf("username :%s\n", message.username);
        //printf("password :%s\n", message.password);
        //printf("msg_type :%d\n", message.message_type);
        sprintf(intToChar, "%d", message.message_type);

        sendto(sockfd, message.message, sizeof(message.message), 0, (struct sockaddr *)&addr, sizeof(addr));
        sendto(sockfd, message.username, sizeof(message.username), 0, (struct sockaddr *)&addr, sizeof(addr));
        sendto(sockfd, message.password, sizeof(message.password), 0, (struct sockaddr *)&addr, sizeof(addr));
        sendto(sockfd, intToChar, sizeof(intToChar), 0, (struct sockaddr *)&addr, sizeof(addr));
        bzero(buff, MAX);
        addr_size = sizeof(addr);

        recvfrom(sockfd, buff, MAX, 0, (struct sockaddr *)&addr, &addr_size);
        responceManager(buff, message.message, message.username, message.password, &message.message_type);
    }
    return 0;
}