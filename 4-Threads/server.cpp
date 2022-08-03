#include <iostream> // std::cout, std::endl
#include <thread>	// std::this_thread::sleep_for
#include <chrono>	// std::chrono::seconds
#ifdef _WIN32
#include <Windows.h>
#else
#endif

#include <stdio.h>
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
#include <string>
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



void ocall_print(const char *str)
{
	std::cout << "ocall:" << std::endl;
	std::cout << str << std::endl;
	return;
}

int ocall_readRequest(int entry, int fd, size_t message_len)
{

	int repint;
	char rep[80];
	read(fd, rep, sizeof(entry));
	repint = atoi(rep);
	std::cout << repint << std::endl;
	return repint;
}

int ocall_TransformInt(const char *str)
{
	std::cout << "HERE" << std::endl;

	std::string s = str;
	s.pop_back();

	std::string str1("1");
	std::string str2("2");
	std::string str3("3");
	std::string str4("4");
	std::string str5("5");
	std::string str6("6");
	std::string str7("7");

	if (!s.compare(str1) != 0)
	{
		return 1;
	}
	if (!s.compare(str2) != 0)
	{
		return 2;
	}
	if (!s.compare(str3) != 0)
	{
		return 3;
	}
	if (!s.compare(str4) != 0)
	{
		return 4;
	}
	if (!s.compare(str5) != 0)
	{
		return 5;
	}
	if (!s.compare(str6) != 0)
	{
		return 6;
	}
	if (!s.compare(str7) != 0)
	{
		return 7;
	}
	return 0;
}

// int sendfile();
static int vsock_listen()
{
	int listen_fd;
	int client_fd;
	struct sockaddr_vm sa_listen = {
		.svm_family = AF_VSOCK,
		.svm_cid = VMADDR_CID_ANY,
	};
	struct sockaddr_vm sa_client;
	socklen_t socklen_client = sizeof(sa_client);

	sa_listen.svm_port = PORT;

	listen_fd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (listen_fd < 0)
	{
		perror("socket");
		return -1;
	}

	if (bind(listen_fd, (struct sockaddr *)&sa_listen, sizeof(sa_listen)) != 0)
	{
		perror("bind");
		close(listen_fd);
		return -1;
	}

	if (listen(listen_fd, 1) != 0)
	{
		perror("listen");
		close(listen_fd);
		return -1;
	}

	client_fd = accept(listen_fd, (struct sockaddr *)&sa_client, &socklen_client);
	if (client_fd < 0)
	{
		perror("accept");
		close(listen_fd);
		return -1;
	}

	fprintf(stderr, "Connection from cid %u port %u...\n", sa_client.svm_cid, sa_client.svm_port);
	close(listen_fd);
	return client_fd;
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

void ecall_makeEntry(char *message, char response[80], size_t message_len)
{

    int num;
    ocall_TransformInt(&num, message);
    // strncpy(response, "aaaaaaaaaaaa", 80);

    switch (num)
    {
    case 1:
        strncpy(response, "poached egg made", 80);
        // ocall_print(response);
        break;
    case 2:
        strncpy(response, "Cucumber made", 80);
        // ocall_print(message);

        break;
    case 3:
        strncpy(response, "Taro and chicken mince made", 80);
        // ocall_print(response);
        break;
    case 4:
        strncpy(response, "Tofu rissoles made", 80);
        // ocall_print(response);
        break;
    case 5:
        strncpy(response, "Miso soup made", 80);
        // ocall_print(response);
        break;
    default:
        strncpy(response, "failed", 80);
        // ocall_print(response);
    }
}


int ecall_entryverif(int entry, int fd, size_t message_len)
{
    int rep;
    ocall_readRequest(&rep, entry, fd, message_len);
    
    
    switch (rep)
    {
    case 1:
        if (!entry1 == 0)
            return 1;
        break;
    case 2:
        if (!entry2 == 0)
            return 1;
        break;
    case 3:
        if (!entry3 == 0)
            return 1;
        break;
    case 4:
        if (!entry4 == 0)
            return 1;
        break;
    case 5:
        if (!entry5 == 0)
            return 1;
        break;
    default:
        return 2;
        break;
        }
    return 2;
}


static void main_loop(int remote_fd)
{
	char buff[80];
	char buffrep[80];
	char debug[1000];
	int fd = remote_fd;
	int n = 0;
	int entry;
	char rep[80];
	int voidrep;
	int check = 1;

	for (;;)
	{
		voidrep = ecall_entryverif(entry, fd, sizeof(entry));
		read(fd, debug, 1000);
		if (voidrep == 1)
		{
			strcpy(rep, "1");
			write(fd, rep, sizeof(buff));
			break;
		}
		else
		{
			strcpy(rep, "2");
			write(fd, rep, sizeof(buff));
		}

	}

	// get waiting data
	//read(fd, debug, 1000);

	if (check == 1)
	{
		n += 1;
		read(fd, buff, sizeof(buff));
		cout << "Client order: " << buff << endl;
		// cout << "aaaaaaaaaaaaa " << buff[0] << endl;
		ecall_makeEntry(&buff[0], rep, sizeof(rep));
		cout << "REP: " << endl;
		cout << rep << endl;
		// ISSUE THERE, I CAN T GET THE DATA I WANT
		// strcpy(buffrep, "Here is your order !! ");

		write(fd, rep, sizeof(buff));
		if (n == 2)
		{
			n = 0;
			// break;
		}
	}
}

/* Application entry */
int main(int argc, char *argv[])
{
	int rep;
	const char *msg = "aa";
	(void)argc;
	(void)argv;

	int remote_fd = vsock_listen();

	if (remote_fd < 0)
	{
		return EXIT_FAILURE;
	}

	write_file(remote_fd);
	cout << "MAIN LOOP" << endl;
	cout << endl;
	main_loop(remote_fd);

	return 0;
}