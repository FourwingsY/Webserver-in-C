#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "send.h"

#define BUF_SIZE 4096
#define SMALL_BUF 200
#define SOCKET_CONTINUE 1
#define SOCKET_DISCONNECT 0

void* socket_handler(void *arg);
int request_handler(int sock);
char* content_type(char *file);
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
int request_handler(int sock)
{
	FILE *clnt_readf = fdopen(dup(sock), "r");
	int clnt_write;
	int str_len = 0, line = 0, i;
	char request[BUF_SIZE];
	char header[20][SMALL_BUF];
	char method[10], con_type[15], file_name[30];
	char contents[BUF_SIZE];

	// GET REQUEST
	memset(request, 0, BUF_SIZE);
	while (1) {
		fgets(request, BUF_SIZE, clnt_readf);
		str_len = strlen(request);

		// CLIENT CLOSED SOCKET OR READ ERROR
		if (str_len == 0) {
			fclose(clnt_readf);
			return SOCKET_DISCONNECT;
		}
		// END OF HEADER
		if (strcmp(request, "\r\n") == 0) {
			fclose(clnt_readf);
			break;
		}

		strcpy(header[line], request);
		// printf("%d, %d, %d, %s", sock, line, str_len, header[line]);
		line++;
		if (line >= 20)
			break;
	}

	// MAKE RESPONSE
	clnt_write = dup(sock);

	// RESPONSE FOR NON-HTTP REQUEST
	if (strstr(header[0], "HTTP/") == 0) {
		send_error(clnt_write);
		close(clnt_write);
		return SOCKET_CONTINUE;
	}

	// IS THIS CLOSE SIGNAL?
	for (i = 0; i < line; i++) {
		if (strstr(header[i], "Connection") != NULL)
			if (strstr(header[i], "close") != NULL) {
				printf("CLOSE signal from %d\n", sock);
				close(clnt_write);
				return SOCKET_DISCONNECT;
			}
	}

	// READ FIRST LINE
	printf("REQUEST: %s", header[0]);
	strcpy(method, strtok(header[0], " /"));
	strcpy(file_name, strtok(NULL, " /"));
	strcpy(con_type, content_type(file_name));
	// FOR INDEX PAGE
	if (strcmp(file_name, "") == 0)
		strcpy(file_name, "index.html");

	// GET
	if (strcmp(method, "GET") == 0)
		send_data(clnt_write, con_type, file_name);
	// POST
	if (strcmp(method, "POST") == 0)
		send_error(clnt_write);

	close(clnt_write);
	return SOCKET_CONTINUE;
}

char* content_type(char *file)
{
	char extension[SMALL_BUF];
	char file_name[SMALL_BUF];
	strcpy(file_name, file);
	strtok(file_name, ".");
	strcpy(extension, strtok(NULL, "."));

	if (strcmp(extension, "html") == 0 || strcmp(extension, "htm") == 0)
		return "text/html";
	if (strcmp(extension, "css") == 0)
		return "text/css";
	if (strcmp(extension, "js") == 0)
		return "application/x-javascript";
	if (strcmp(extension, "ico") == 0)
		return "image/x-icon";
	if (strcmp(extension, "png") == 0)
		return "image/png";
	return "text/plain";
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}