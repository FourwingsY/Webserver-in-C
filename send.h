#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 4096
#define SMALL_BUF 200

void send_data(int clnt_write, char* type, char* file_name);
void send_error(int clnt_write);