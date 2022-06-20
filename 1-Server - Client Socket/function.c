#include "functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
struct message
{
    char message[MAX];
    int message_type; // 1: message, 2: login, 3: logout, 4: help
    char username[MAX];
    char password[MAX];
};
*/
/*

void responceManager(char buff[MAX], struct message *message)
{
    int response;
    response = atoi(buff);
    printf("[RESPONCE-MANAGER] Response: %d \n", response);
    switch (response)
    {
    case 999999:
        printf("help commands: \n");
        printf("Message : ");
        message->message_type = 4;
        break;
    case 999998:
        char username[20], password[30];
        printf("USERNAME : ");
        scanf("%s", message->username);
        printf("PASSWORD : ");
        scanf("%s", message->password);
        message->message_type = 2;
        break;
    case 999997: // client leaves
        printf("Client Leaves \n");
        message->message_type = 3;
        break;
    default:
        printf("Message : ");
        message->message_type = 1;
        break;
    }
}


int decodeBuffer(char buff[MAX])
{

    if (strcmp(buff, "-help\n") == 0)
    {
        return 999999;
    }
    else if (strcmp(buff, "-login\n") == 0)
    {
        return 999998;
    }
    else if (strcmp(buff, "-exit\n") == 0)
    {
        return 999997;
    }
    else
        return 1;
}

int RequestManager(char buff[MAX])
{
    int decoded = decodeBuffer(buff);
    switch (decoded)
    {
    case 999999:
        printf("[REQUEST-MANAGER] Help Request\n");
        return 999999;
        break;

    case 999998:
        printf("[REQUEST-MANAGER] Client wants to connect\n");
        return 999998;
        break;

    case 999997:
        printf("[REQUEST-MANAGER] Client leaves\n");
        return 999997;
        break;

    default:
        printf("[REQUEST-MANAGER] Not a request message \n");
        return 0;
        break;
    }
}*/