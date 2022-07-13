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
#include <iostream>

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

void ocall_print_menu(void)
{
    std::cout << "Menu:" << std::endl;
    cout << endl;
    std::cout << "aaaaaaaaaaaaaa" << std::endl;
    std::cout << "nnnnnnnnnnnnnn" << std::endl;
    std::cout << "gggggggggggggg" << std::endl;
    std::cout << "aaaaaaaaaaaaaa" << std::endl;
    std::cout << "aaaaaaaaaaaaaa" << std::endl;
    std::cout << "aaaaaaaaaaaaaa" << std::endl;
    return;
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
        .svm_cid = 3, //2
        .svm_flags = 0,
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

void write_file(int remote_fd)
{

    size_t n;
    char buffer[SIZE];
    int index = 0;
    // cout << "-----writing on file:" << endl;
    FILE *fp = fopen("rez.so", "w");
    while (1)
    {
        n = read(remote_fd, buffer, sizeof(buffer));
        // cout << n << endl;
        fwrite(buffer, sizeof(char), n, fp);
        // cout << buffer << endl;
        index += 1;
        // cout << index << endl;
        if (n < 1000)
        {
            break;
        }
    }

    fclose(fp);
}

static void main_loop(int remote_fd)
{
    char buff[80];
    int fd = remote_fd;
    char message[200];
    int retval;
    int finished = 0;

    strcpy(message, "A menu is composed of a an entrée, a main course and a dessert");
    cout << "Welcome in Kyutech restaurant! " << endl;
    for (;;)
    {
        // loop where client choose the dish
        for (;;)
        {
            ecall_print_char(global_eid, &retval, message, sizeof(message));
            // entrée loop
            while (finished == 0)
            {
                ecall_call_menu(global_eid, &retval);
                strcpy(message, "What do you awant to take?");
                cout << " => "  << endl;
                sscanf(buff, "%80[^\n]");
                strcpy(message, "Is there anything else you want?");
                // get answer
                // if (answer == 0)
                finished = 1;
            }
            finished = 0;
            // ecall_call_menu(global_eid, &retval);

            break;
        }
        // MENU CHOOSEN HERE
        write(fd, buff, sizeof(buff));

        /*
        //Sending menu to server!
        //bzero(buff, sizeof(buff));
        //printf("Enter the string : ");
        //n = 0;
        //while ((buff[n++] = getchar()) != '\n')
        //	;
        write(fd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));*/
        read(fd, buff, sizeof(buff));
        printf("From Server : %s", buff);

        break;
    }
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
    write_file(remote_fd);
    // cout << "MAIN LOOP" << endl;

    /* Initialize the enclave */
    if (initialize_enclave() < 0)
    {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1;
    }
    // cout << "Enclave created" << endl;
    cout << endl;
    main_loop(remote_fd);

    sgx_destroy_enclave(global_eid);
    return EXIT_SUCCESS;
}

/*
static int data(int in_fd, int out_fd)
{
    char buf[4096];
    char *send_ptr = buf;
    ssize_t nbytes;
    ssize_t remaining;

    nbytes = read(in_fd, buf, sizeof(buf));
    if (nbytes <= 0)
    {
        return -1;
    }

    remaining = nbytes;
    while (remaining > 0)
    {
        nbytes = write(out_fd, send_ptr, remaining);
        if (nbytes < 0 && errno == EAGAIN)
        {
            nbytes = 0;
        }
        else if (nbytes <= 0)
        {
            return -1;
        }

        if (remaining > nbytes)
        {
            // Wait for fd to become writeable again
            for (;;)
            {
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(out_fd, &wfds);
                if (select(out_fd + 1, NULL, &wfds, NULL, NULL) < 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    else
                    {
                        perror("select");
                        return -1;
                    }
                }

                if (FD_ISSET(out_fd, &wfds))
                {
                    break;
                }
            }
        }

        send_ptr += nbytes;
        remaining -= nbytes;
    }
    return 0;
}
*/