#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define MAX 200
#define MAXUSER 30
#define PORT 7700


struct message
{
    char message[MAX];
    int message_type; // 1: message, 2: login, 3: logout, 4: help
    char username[MAX];
    char password[MAX];
};
/*
void responceManager(char buff[MAX], struct message *message);
*/
//int RequestManager(char buff[MAX]);
//int decodeBuffer(char buff[MAX]);
#endif // FUNCTIONS_HZ