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
/*
#include <sgx_urts.h>
#include <sgx_uswitchless.h>
#include "App.h"
#include "Enclave_u.h"
*/
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
#define PORT 7777
#define SIZE 1024


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
        .svm_cid = 3, // 2
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

void send_file(int remote_fd)
{

	int n, bytes;
	int j = 0;
	FILE *f;
	char buffer[1000];
	char const *pchar;
	f = fopen("enclave.signed.so", "rb");
	cout << "-----writting on file:" << endl;
	while (!feof(f))
	{
		bytes = fread(buffer, 1, 1000, f);
		write(remote_fd, buffer, bytes);

		j += 1;
		//cout << j << endl;
		//cout << buffer << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	fclose(f);
}

static void main_loop(int remote_fd)
{
    char buff[80];
    char order[200];
    int fd = remote_fd;
    char message[200];
    int retval, n;
    int finished = 0;
    int checkValid = 0;
    strcpy(message, "A menu is composed of a an entrée, a main course and a dessert");
    cout << "\nWelcome in Kyutech restaurant! " << endl;
    for (;;)
    {
        // loop where client choose the dish

        ecall_print_char(global_eid, &retval, message, sizeof(message));
        // entrée loop
        while (finished == 0)
        {
            checkValid = 0;
            // ecall_call_menu(global_eid, &retval);
            strcpy(message, " \nWhat entry do you want to take?");
            cout << message << endl;
            ecall_call_entry(global_eid, &retval);
            cout << " => ";
            bzero(buff, 200);
            n = 0;
            while ((buff[n++] = getchar()) != '\n')
                ;
            // check response

            strcpy(message, " \nWhat main dish do you want to take?");
            cout << message << endl;
            ecall_call_maindish(global_eid, &retval);
            cout << " => ";
            bzero(buff, 200);
            n = 0;
            while ((buff[n++] = getchar()) != '\n')
                ;
            // check response

            strcpy(message, " \nWhat entry do you want to take?");
            cout << message << endl;
            ecall_call_dessert(global_eid, &retval);
            cout << " => ";
            bzero(buff, 200);
            n = 0;
            while ((buff[n++] = getchar()) != '\n')
                ;
            // check response

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
        }
        cout << "it's time to send data" << endl;
        // MENU CHOOSEN HERE
        strcpy(order, "1 2 3");
        write(fd, order, sizeof(order));

        //---------------------
        read(fd, buff, sizeof(buff));
        printf("From Server : %s", buff);

        break;
    }
    cout << "Bye bye" << endl;
}


int  main(int argc, char *argv[])
{

    (void)argc;
    (void)argv;

    int remote_fd = vsock_connect();
    if (remote_fd < 0)
    {
        return EXIT_FAILURE;
    }
    send_file(remote_fd);
    //  cout << "MAIN LOOP" << endl;


    // cout << "Enclave created" << endl;
    cout << endl;
    main_loop(remote_fd);


    return 0;
}
