#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "send.h"
#include "request_handler.h"
#include "constants.h"

void* socket_handler(void *arg);
void error_handling(char *msg);

int main(int argc, const char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	unsigned int addr_sz;
	pthread_t tid;

	if (argc != 2) {
		printf("Usage : %s <Port> \n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while (1) {
		addr_sz = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &addr_sz);
		printf("Connection request : %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

		// accept -> clnt sock is argument of new thread
		pthread_create(&tid, NULL, socket_handler, (void*) &clnt_sock);
		pthread_detach(tid);
	}
	printf("SERVER EXIT");
	close(serv_sock);
	return 0;
}

void* socket_handler(void *arg)
{
	int sock = *((int*)arg);
	int result = SOCKET_CONTINUE;

	printf("SOCKET %d CONNECTED\n", sock);
	while (result == SOCKET_CONTINUE) {
		result = request_handler(sock);
	}
	printf("SOCKET %d CLOSED\n", sock);
	close(sock);
	return NULL;

}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}