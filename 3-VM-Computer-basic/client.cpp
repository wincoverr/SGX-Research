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

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
#define PORT 7777
#define SIZE 1024

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
            // ecall_print_char(global_eid, &retval, message, sizeof(message));
            // entrée loop
            while (finished == 0)
            {
                //   ecall_call_menu(global_eid, &retval);
                strcpy(message, "What do you awant to take?");
                cout << " => " << endl;
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
        read(fd, buff, sizeof(buff));
        printf("From Server : %s", buff);

        break;
    }
    cout << "Bye bye" << endl;
}

/* Application entry */
int main(int argc, char *argv[])
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

    cout << "Enclave created" << endl;
    cout << endl;
    main_loop(remote_fd);

    return 0;
}
