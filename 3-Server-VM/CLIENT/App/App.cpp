/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <linux/vm_sockets.h>
#include <unistd.h>
#include <pwd.h>
#include <iostream> // std::cout, std::endl
#include <thread>   // std::this_thread::sleep_for
#include <chrono>   // std::chrono::seconds

#include <sgx_urts.h>
#include <sgx_uswitchless.h>
#include "App.h"
#include "Enclave_u.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
#define PORT 7777
#define SIZE 1024

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t
{
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

#define REPEATS 500000

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
    {SGX_ERROR_MEMORY_MAP_FAILURE,
     "Failed to reserve memory for the enclave.",
     NULL},
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist / sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++)
    {
        if (ret == sgx_errlist[idx].err)
        {
            if (NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
        printf("Error: Unexpected error occurred.\n");
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS)
    {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

void ocall_print(const char *str)
{
    std::cout << str << std::endl;
    return;
}

void ocall_print_entry(void)
{
    std::cout << "Entry:" << std::endl;
    cout << endl;
    std::cout << "1 - poached egg" << std::endl;
    std::cout << "2 - Cucumber" << std::endl;
    std::cout << "3 - Taro and chicken mince " << std::endl;
    std::cout << "4 - Tofu Rissoles" << std::endl;
    std::cout << "5 - miso soup" << std::endl;
    return;
}

void ocall_print_maindish(void)
{
    std::cout << "Main dish:" << std::endl;
    cout << endl;
    std::cout << "1 - Udon" << std::endl;
    std::cout << "2 - Okonomiyaki" << std::endl;
    std::cout << "3 - Pork Cutlet" << std::endl;
    std::cout << "4 - Karaage" << std::endl;
    std::cout << "5 - Sushi" << std::endl;
    std::cout << "6 - Kiritampo rolled with sliced pork" << std::endl;
    return;
}

void ocall_print_dessert(void)
{
    std::cout << "Dessert:" << std::endl;
    cout << endl;
    std::cout << "1 - Parfait" << std::endl;
    std::cout << "2 - Castella" << std::endl;
    std::cout << "3 - Ice Cream" << std::endl;
    std::cout << "4 - Wagashi" << std::endl;
    std::cout << "5 - crepes" << std::endl;
    return;
}

int ocall_sendRequest(int entry, int fd, size_t message_len)
{
    char rep[80];
    std::string s = std::to_string(entry);
    char const *pchar = s.c_str();
    write(fd, pchar, sizeof(pchar));
    read(fd, rep, sizeof(rep));
    std::string str(rep);
    int repINT = atoi(str.c_str());
    if (repINT == 1)
    {
        std::cout << "This entry can be made" << std::endl;
    }
    else
    {
        std::cout << "This entry can't be made; choose something else please" << std::endl;
   
    }
    return repINT;
}
static int vsock_connect()
{
    int fd;
    int cid;
    int port;
    socklen_t addr_size;
    struct sockaddr_vm sa = {
        .svm_family = AF_VSOCK,
        .svm_reserved1 = 0,
        .svm_port = PORT,
        .svm_cid = 2, // 2
    };

    fd = socket(AF_VSOCK, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
    {
        perror("connect");
        close(fd);
        return -1;
    }
    return fd;
}

void send_file(int remote_fd)
{

    int n, bytes;
    int j = 0;
    FILE *f;
    char buffer[1000];
    char const *pchar;
    f = fopen("enclave.signed.so", "rb");
    cout << "-----sending file:" << endl;
    while (!feof(f))
    {
        bytes = fread(buffer, 1, 1000, f);
        write(remote_fd, buffer, bytes);

        j += 1;
        // cout << j << endl;
        // cout << buffer << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    fclose(f);
}

static void main_loop(int remote_fd)
{
    char buff[200];
    char order[200];
    char buffOrder[80];
    int fd = remote_fd;
    char message[200];
    int retval, n;
    int finished = 2;
    int checkValid = 0;
    int entry;
    cout << "\nWelcome in Kyutech restaurant! " << endl;
    for (;;)
    {
        // loop where client choose the dish

        // ecall_print_char(global_eid, &retval, message, sizeof(message));
        //  entrée loop
        while (finished == 2)
        {
            checkValid = 0;
            // ecall_call_menu(global_eid, &retval);
            strcpy(message, " \nWhat entry do you want to take?");
            cout << message << endl;
            ecall_call_entry(global_eid, &retval);
            cout << " => ";
            bzero(buffOrder, 200);
            n = 0;
            while ((buffOrder[n++] = getchar()) != '\n')
                ;

            entry = atoi(buffOrder);
            ecall_requestEntryLeft(global_eid,&finished, entry, fd, sizeof(entry));
/*
            while (checkValid == 0)
            {
                strcpy(message, "Is this order fine for you? \n y:yes \n n : no \n =>");
                cout << message;
                bzero(buff, 200);
                n = 0;
                while ((buff[n++] = getchar()) != '\n')
                    ;
                cout << &buff[0] << endl;
                if ((int)buff[0] == 121 || (int)buff[0] == 110)
                    checkValid = 1;
            }
            if ((int)buff[0] == 121)
                break;
                */
        }
        cout << "it's time to send data" << endl;
        // MENU CHOOSEN HERE
        write(fd, buffOrder, sizeof(buffOrder));

        //---------------------
        read(fd, buff, sizeof(buff));
        printf("From Server : %s", buff);

        break;
    }
    cout << endl;
    cout << "Bye bye" << endl;
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{

    (void)argc;
    (void)argv;

    int remote_fd = vsock_connect();
    if (remote_fd < 0)
    {
        return EXIT_FAILURE;
    }
    send_file(remote_fd);

    /* Initialize the enclave */
    if (initialize_enclave() < 0)
    {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1;
    }
    cout << "Enclave created" << endl;
    cout << endl;
    cout << "MAIN LOOP" << endl;
    main_loop(remote_fd);

    sgx_destroy_enclave(global_eid);
    return EXIT_SUCCESS;
}
