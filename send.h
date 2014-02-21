#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "url_encode.h"

#define BUF_SIZE 4096
#define SMALL_BUF 200
#define WEBAPP_DIR "webapp"

void send_data(int clnt_write, char* type, char* file_name);
void send_error(int clnt_write);
void send_dir_page(int clnt_write, char* serv_path);
void send_chunked(int clnt_write, char* chunked_data);
void send_chunked_end(int clnt_write);