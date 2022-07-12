#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <linux/vm_sockets.h>
#include <thread>
#define PORT 7777
#define SIZE 1024
#include <iostream>
using namespace std;
#include <iostream> // std::cout, std::endl
#include <thread>	// std::this_thread::sleep_for
#include <chrono>	// std::chrono::seconds
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

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
/*
void send_file(int remote_fd)
{

	int n, bytes;
	int j = 0;
	string document;
	FILE *f, *writee;
	char buff[1000];

	// cout << "aaaaaaaa";
	// writee = fopen ("client.so", "wb");
	f = fopen("client.so", "rb");
	char *buffer[1000];

	char const *pchar;
	std::string s;
	while (!feof(f))
	{
		sleep(0.1);
		bytes = fread(buffer, 1, 1000, f);

		s = std::to_string(bytes);
		pchar = s.c_str();

		cout << pchar << endl;
		write(remote_fd, pchar, sizeof(pchar));
		write(remote_fd, buffer, bytes);
		j += 1;
		cout << j << endl;
	}
	cout << "aaaaaaaa" << endl;
	fclose(f);
}*/
void send_file(int remote_fd)
{

	int n, bytes;
	int j = 0;
	FILE *f;
	char buffer[1000];
	char const *pchar;
	f = fopen("client.so", "rb");
	cout << "-----writting on file:" << endl;
	while (!feof(f))
	{
		bytes = fread(buffer, 1, 1000, f);
		write(remote_fd, buffer, bytes);

		j += 1;
		//cout << j << endl;
		//cout << buffer << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	fclose(f);
}

static void main_loop(int remote_fd)
{
	fd_set rfds;
	int nfds = remote_fd + 1;
	char buff[80];
	int fd = remote_fd;
	int n;

	for (;;)
	{
		// write(fd, buff, sizeof(buff));

		n = n + 1;

		read(fd, buff, sizeof(buff));
		cout << "Client order: " << buff << endl;
		// ToDo: read order

		///////////-----

		// make dishes

		// send dishes
		//   printf("From client: %s\t", buff);
		/*
		bzero(buff, 80);
		n = 0;

		while ((buff[n++] = getchar()) != '\n')
			;
		*/
		strcpy(buff, "Here is your order !! ");

		write(fd, buff, sizeof(buff));
		if (n == 2)
		{
			n = 0;
			break;
		}
	}
}

int main(int argc, char **argv)
{
	for (;;)
	{
		int remote_fd = vsock_listen();

		if (remote_fd < 0)
		{
			return EXIT_FAILURE;
		}
		send_file(remote_fd);
		cout << "MAIN LOOP" << endl;
		main_loop(remote_fd);
	}
}
