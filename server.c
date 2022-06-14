#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "functions.h"

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
}

int connexion(char pseudo[MAX], char mdp[MAX])
{

    FILE *fichier = NULL;
    fichier = fopen("users.txt", "r");
    if (fichier == NULL)
    {
        printf("Erreur d'ouverture du fichier users.txt\n");
        return 0;
    }
    char buff[MAX];
    char username[MAX];
    char password[MAX];
    int i = 0;
    while (fgets(buff, MAX, fichier) != NULL)
    {
        sscanf(buff, "%s %s", username, password);
        if (strcmp(username, pseudo) == 0 && strcmp(password, mdp) == 0)
        {
            printf("Connexion réussie\n");
            return 1;
        }
        i++;
    }
    printf("Connexion échouée\n");
    return 0;
}

int main(int argc, char **argv)
{
    char temp[MAX];
    struct message message;
    char *ip = "127.0.0.1";
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[MAX];
    char response[MAX];
    socklen_t addr_size;
    int n;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("[-]socket error");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]bind error");
        exit(1);
    }
    else
        printf("[+]bind success, listening\n");

    for (;;)
    {

        bzero(buffer, MAX);
        addr_size = sizeof(client_addr);
        /*        sendto(sockfd, message.message, sizeof(message.message), 0, (struct sockaddr *)&addr, sizeof(addr));
        sendto(sockfd, message.username, sizeof(message.username), 0, (struct sockaddr *)&addr, sizeof(addr));
        sendto(sockfd, message.password, sizeof(message.password), 0, (struct sockaddr *)&addr, sizeof(addr));
        sendto(sockfd, intToChar, sizeof(intToChar), 0, (struct sockaddr *)&addr, sizeof(addr));
        */
        recvfrom(sockfd, message.message, sizeof(message.message), 0, (struct sockaddr *)&client_addr, &addr_size);
        recvfrom(sockfd, message.username, sizeof(message.username), 0, (struct sockaddr *)&client_addr, &addr_size);
        recvfrom(sockfd, message.password, sizeof(message.password), 0, (struct sockaddr *)&client_addr, &addr_size);
        recvfrom(sockfd, temp, sizeof(temp), 0, (struct sockaddr *)&client_addr, &addr_size);
        message.message_type = atoi(temp);
        // printf("msg : %s", data[0]);
        // printf("username :%s\n", data[1]);
        // printf("password :%s\n", data[2]);
        printf("msg_type :%d\n", message.message_type);

        connexion(message.username, message.password);

        int decoded = RequestManager(message.message);
        sprintf(response, "%d", decoded);
        sendto(sockfd, response, MAX, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }
    return 0;
}
