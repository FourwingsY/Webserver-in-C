#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "send.h"
#include "request_handler.h"
#include "constants.h"

int read_header(FILE *clnt_read, char header[HEADER_MAX][SMALL_BUF], int *header_len);
void read_contents(FILE *clnt_read, char contents[BUF_SIZE], int *cont_len);
char* content_type(char *file);

int request_handler(int sock)
{
	FILE *clnt_readf = fdopen(dup(sock), "r");
	int clnt_write;
	int header_len = 0, cont_len = 0;
	int i, result;
	char header[HEADER_MAX][SMALL_BUF];
	char method[10], con_type[100], file_name[SMALL_BUF];
	char contents[BUF_SIZE];

	// HEADER
	result = read_header(clnt_readf, header, &header_len);
	if (result == SOCKET_DISCONNECT) {
		fclose(clnt_readf);
		return SOCKET_DISCONNECT;
	}

	// MAKE RESPONSE
	clnt_write = dup(sock);

	// RESPONSE FOR NON-HTTP REQUEST
	if (strstr(header[0], "HTTP/") == NULL) {
		send_error(clnt_write);
		close(clnt_write);
		return SOCKET_CONTINUE;
	}

	// READ FIRST LINE of REQUEST
	printf("REQUEST: %s", header[0]);
	strcpy(method, strtok(header[0], " "));
	strcpy(file_name, strtok(NULL, " "));
	strcpy(con_type, content_type(file_name));

	// LINKING PAGE to FUNCTION
	if (strcmp(file_name, "/") == 0)
		strcpy(file_name, "/index.html");
	if (strstr(file_name, "/webhard") != NULL) {
		send_dir_page(clnt_write, file_name);
		close(clnt_write);
		return SOCKET_CONTINUE;
	}

	// GET
	if (strcmp(method, "GET") == 0)
		send_data(clnt_write, con_type, file_name);
	// POST
	if (strcmp(method, "POST") == 0) {
		// GET THE CONTENT-LENGTH
		for (i = 0; i < header_len; i++) {
			if (strstr(header[i], "Content-Length") != NULL)
				break;
		}
		cont_len = atoi(&header[i][15]);
		printf("cont len: %d\n", cont_len);

		read_contents(clnt_readf, contents, &cont_len);

		// AFTER READ POST
		send_data(clnt_write, con_type, file_name);
	}

	fclose(clnt_readf);
	close(clnt_write);
	return SOCKET_CONTINUE;
}

int read_header(FILE *clnt_read, char header[HEADER_MAX][SMALL_BUF], int *header_len)
{
	char request[BUF_SIZE];
	int line = 0, str_len;
	int i;

	memset(request, 0, BUF_SIZE);
	while (1) {
		fgets(request, BUF_SIZE, clnt_read);
		str_len = strlen(request);

		// CLIENT CLOSED SOCKET OR READ ERROR
		if (str_len == 0) {
			fclose(clnt_read);
			return SOCKET_DISCONNECT;
		}
		// END OF HEADER
		if (strcmp(request, "\r\n") == 0) {
			break;
		}

		strcpy(header[line], request);
		printf("%d, %d, %s", line, str_len, header[line]);
		line++;
		if (line >= HEADER_MAX)
			break;
	}

	// CHECK CLOSE SIGNAL
	for (i = 0; i < line; i++) {
		if (strstr(header[i], "Connection") != NULL && strstr(header[i], "close") != NULL) {
			return SOCKET_DISCONNECT;
		}
	}

	return SOCKET_CONTINUE;
}

void read_contents(FILE *clnt_read, char contents[BUF_SIZE], int *cont_len)
{
	int str_len;
	// READ POST MESSAGE BODY
	// INCLUDING NULL -> +1
	memset(contents, 0, BUF_SIZE);
	fgets(contents, *cont_len + 1, clnt_read);
	str_len = strlen(contents);
	printf("post data: %d, %s\n", str_len, contents);
}

char* content_type(char *file)
{
	char extension[SMALL_BUF];
	char file_name[SMALL_BUF];

	if (strstr(file, ".") == NULL) {
		return "wrong";
	}
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