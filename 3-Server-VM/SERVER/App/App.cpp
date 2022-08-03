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

	//	std::cout << b << std::endl;
	/*
	std::cout << "called" << std::endl;
	std::cout << str << std::endl;
	std::cout << str[0] << std::endl;
	std::cout << str[2] << std::endl;
	std::cout << str[4] << std::endl;
	std::cout << "tesssssssssssss" << std::endl;


	std::cout << b <<  std::endl;
	const char *c = &str[0];
	std::cout << c<< std::endl;
	const char *d = &str[2];
	std::cout << d<< std::endl;
*/
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
		ecall_entryverif(global_eid, &voidrep, entry, fd, sizeof(entry));
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
		ecall_makeEntry(global_eid, &buff[0], rep, sizeof(rep));
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
int SGX_CDECL main(int argc, char *argv[])
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

	/* Initialize the enclave */
	if (initialize_enclave() < 0)
	{
		printf("Enter a character before exit ...\n");
		getchar();
		return -1;
	}
	cout << "Enclave created" << endl;
	cout << endl;
	main_loop(remote_fd);

	sgx_destroy_enclave(global_eid);
	return EXIT_SUCCESS;
}