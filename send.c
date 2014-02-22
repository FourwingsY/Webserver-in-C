#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "send.h"
#include "constants.h"

void send_data(int clnt_write, char* type, char* file_name)
{
	char protocol[] = "HTTP/1.1 200 OK\r\n";
	char server[] = "Server:Saik's Linux Web Server\r\n";
	char cont_len[] = "Content-Length:toolong number\r\n";
	char connection[] = "Connection:Keep-Alive\r\n";
	char cont_type[] = "Content-type:temporary/string\r\n";

	char buf[BUF_SIZE];
	char relative_path[SMALL_BUF] = WEBAPP_DIR;
	char file_path[SMALL_BUF];
	int isBinary = 0;
	FILE * send_file;
	int file_length = 0, buf_length;

	// OPEN FILE
	sprintf(file_path, "%s%s", relative_path, file_name);
	printf("File request from socket %d: %s\n", clnt_write, file_name);
	if (strstr(type, "text") == NULL) {
		isBinary = 1;
	}

	if (isBinary) {
		send_file = fopen(file_path, "rb");
	}
	else {
		send_file = fopen(file_path, "r");
	}

	if (send_file == NULL) {
		printf("OPEN ERROR: %s\n", file_path);
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

void send_dir_page(int clnt_write, char* url_path)
{
	DIR *dir;
	struct dirent *dir_entry;
	// struct stat statbuf;
	char webhard_path[SMALL_BUF] = "/Users/YG/Dropbox/Webserver/Project/webhard";
	char sub_path[SMALL_BUF] = "";
	char *dir_name;
	char list_tag[SMALL_BUF];
	char html_start[] = "<html><head></head><body>";
	char html_end[] = "</body></html>";
	
	char protocol[] = "HTTP/1.1 200 OK\r\n";
	char server[] = "Server:Saik's Linux Web Server\r\n";
	char trsf_encoding[] = "Transfer-Encoding: chunked\r\n";
	char connection[] = "Connection:Keep-Alive\r\n";
	char cont_type[] = "Content-type:text/html\r\n";
	

	// GET SUB_PATH
	strcpy(sub_path, &url_path[8]);

	// PARSING PATH
	if (strlen(sub_path) == 0) {
		strcpy(sub_path, "/");
	}

	if (sub_path[strlen(sub_path)-1] != '/') {
		printf("original : %s\n", sub_path);
		sub_path[strlen(sub_path)] = '/';
		sub_path[strlen(sub_path)+1] = '\0';
	}

	sprintf(webhard_path, "%s%s", webhard_path, sub_path);
	
	printf("open : %s\n", webhard_path);

	// HEADER
	write(clnt_write, protocol, strlen(protocol));
	write(clnt_write, server, strlen(server));
	write(clnt_write, trsf_encoding, strlen(trsf_encoding));
	write(clnt_write, cont_type, strlen(cont_type));
	write(clnt_write, connection, strlen(connection));
	write(clnt_write, "\r\n", strlen("\r\n"));

	// HTML START
	send_chunked(clnt_write, html_start);
	
	// SEARCH DIR
	dir = opendir(webhard_path);
	if (dir == NULL) {
		printf("dir open error\n");
		return;
	}

	while ((dir_entry = readdir(dir)) != NULL) {
		dir_name = dir_entry->d_name;
		// TO KNOW DIR or FILE
		// stat(dir_entry->d_name, &statbuf);

		// HIDE . .. and hidden files
		if (dir_name[0] == '.' || strstr(dir_name, "/.") != NULL) {
			printf("hidden dir: %s\n", dir_name);
			continue;
		}
		sprintf(list_tag, "<li><a href='webhard%s%s'>%s</a></li>", sub_path, dir_name, dir_name);
		send_chunked(clnt_write, list_tag);
	}
	closedir(dir);
	send_chunked(clnt_write, html_end);
	send_chunked_end(clnt_write);
	
	close(clnt_write);
}

void send_chunked(int clnt_write, char* chunked_data)
{
	char chunked_len[10];

	sprintf(chunked_len, "%lX\r\n", strlen(chunked_data));

	write(clnt_write, chunked_len, strlen(chunked_len));
	write(clnt_write, chunked_data, strlen(chunked_data));
	write(clnt_write, "\r\n", strlen("\r\n"));
}

void send_chunked_end(int clnt_write)
{
	write(clnt_write, "0\r\n\r\n", 5);
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