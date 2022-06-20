#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
# include <unistd.h>
# include <pwd.h>
#include "Enclave_u.h"
#include "sgx_urts.h"
#include "sgx_uae_service.h"

#define MAX 200
#define MAXUSER 30
#define PORT 7700

#define ENCLAVE_FILENAME "enclave.signed.so"

struct message
{
    char message[MAX];
    int message_type; // 1: message, 2: login, 3: logout, 4: help
    char username[MAX];
    char password[MAX];
};

typedef struct _sgx_errlist_t
{
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {SGX_ERROR_UNEXPECTED,
     "Unexpected error occurred.",
     NULL},
    {SGX_ERROR_INVALID_PARAMETER,
     "Invalid parameter.",
     NULL},
    {SGX_ERROR_OUT_OF_MEMORY,
     "Out of memory.",
     NULL},
    {SGX_ERROR_ENCLAVE_LOST,
     "Power transition occurred.",
     "Please refer to the sample \"PowerTransition\" for details."},
    {SGX_ERROR_INVALID_ENCLAVE,
     "Invalid enclave image.",
     NULL},
    {SGX_ERROR_INVALID_ENCLAVE_ID,
     "Invalid enclave identification.",
     NULL},
    {SGX_ERROR_INVALID_SIGNATURE,
     "Invalid enclave signature.",
     NULL},
    {SGX_ERROR_OUT_OF_EPC,
     "Out of EPC memory.",
     NULL},
    {SGX_ERROR_NO_DEVICE,
     "Invalid SGX device.",
     "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."},
    {SGX_ERROR_MEMORY_MAP_CONFLICT,
     "Memory map conflicted.",
     NULL},
    {SGX_ERROR_INVALID_METADATA,
     "Invalid enclave metadata.",
     NULL},
    {SGX_ERROR_DEVICE_BUSY,
     "SGX device was busy.",
     NULL},
    {SGX_ERROR_INVALID_VERSION,
     "Enclave version was invalid.",
     NULL},
    {SGX_ERROR_INVALID_ATTRIBUTE,
     "Enclave was not authorized.",
     NULL},
    {SGX_ERROR_ENCLAVE_FILE_ACCESS,
     "Can't open enclave file.",
     NULL},
    {SGX_ERROR_NDEBUG_ENCLAVE,
     "The enclave is signed as product enclave, and can not be created as debuggable enclave.",
     NULL},
};

void print_error_message(sgx_status_t ret);
int RequestManager(char buff[MAX]);
int decodeBuffer(char buff[MAX]);

void ocall_print(const char *str)
{
    printf("%s", str);
}

int SGX_CDECL main(int argc, char *argv[])
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
    sgx_launch_token_t token = {0};
    sgx_enclave_id_t eid;
    int updated;
    sgx_status_t ret;

    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token,
                             &updated, &eid, NULL);
    if (ret != SGX_SUCCESS)
    {
        printf("cannot create enclave: %d\n", ret);
        return -1;
    }

    ret = ecall_test(eid);
    if (ret != SGX_SUCCESS)
    {
        printf("ecall_ids error: %d\n", ret);
        return -1;
    }

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

        //encrypt message.username and message.password
    
        ecall_connexion(eid, message.username, message.password);
        ecall_meteo(eid, "Paris");

        int decoded = RequestManager(message.message);
        sprintf(response, "%d", decoded);
        sendto(sockfd, response, MAX, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }

    sgx_destroy_enclave(eid);

    return 0;
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
}



void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
        printf("Error: Unexpected error occurred.\n");
}