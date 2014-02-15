#include "send.h"

void send_data(int clnt_write, char* type, char* file_name)
{
	char protocol[] = "HTTP/1.1 200 OK\r\n";
	char server[] = "Server:Saik's Linux Web Server\r\n";
	char cont_len[] = "Content-Length:toolong number\r\n";
	char connection[] = "Connection:Keep-Alive\r\n";
	char cont_type[] = "Content-type:temporary/string\r\n";

	char buf[BUF_SIZE];
	int isBinary = 0;
	FILE * send_file;
	int file_length = 0, buf_length;

	// OPEN FILE
	printf("File request from socket %d: %s\n", clnt_write, file_name);
	if (strstr(type, "text") == NULL) {
		isBinary = 1;
	}

	if (isBinary) {
		send_file = fopen(file_name, "rb");
	}
	else {
		send_file = fopen(file_name, "r");
	}

	if (send_file == NULL) {
		printf("OPEN ERROR: %s\n", file_name);
		send_error(clnt_write);
		return;
	}
	
	// GET FILE LENGTH
	fseek(send_file, 0, SEEK_END);
	file_length = ftell(send_file);
	fseek(send_file, 0, SEEK_SET);

	// EDIT HEADER
	sprintf(cont_type, "Content-type:%s\r\n", type);
	sprintf(cont_len, "Content-Length:%d\r\n", file_length);

	// SEND HEADER
	write(clnt_write, protocol, strlen(protocol));
	write(clnt_write, server, strlen(server));
	write(clnt_write, cont_len, strlen(cont_len));
	write(clnt_write, cont_type, strlen(cont_type));
	write(clnt_write, connection, strlen(connection));
	write(clnt_write, "\r\n", strlen("\r\n"));

	// READ FILE
	// SEND CONTENT
	if (isBinary) {
		while (0 < (buf_length = fread(buf, 1, BUF_SIZE, send_file))) {
			write(clnt_write, buf, BUF_SIZE);
		}
	}
	else {
		while (fgets(buf, BUF_SIZE, send_file) != NULL) {
			write(clnt_write, buf, strlen(buf));
		}
	}

	fclose(send_file);
}


void send_error(int clnt_write)
{
	char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
	char server[] = "Server:Saik's Linux Web Server\r\n";
	char cont_len[] = "Content-length:148\r\n";
	char cont_type[] = "Content-type:text/html \r\n";
	char connection[] = "Connection:Keep-Alive\r\n";

	char content[] = "<html><head><title>NETWORK</title></head>"
	"<body><font size=5><br>Error! Check your request file name is correct or Method is GET</font></body></html>";

	write(clnt_write, protocol, strlen(protocol));
	write(clnt_write, server, strlen(server));
	write(clnt_write, cont_len, strlen(cont_len));
	write(clnt_write, cont_type, strlen(cont_type));
	write(clnt_write, connection, strlen(connection));
	write(clnt_write, "\r\n", strlen("\r\n"));

	write(clnt_write, content, strlen(content));

	close(clnt_write);
}