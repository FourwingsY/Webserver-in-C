#define WEBAPP_DIR "webapp"

void send_data(int clnt_write, char* type, char* file_name);
void send_error(int clnt_write);
void send_dir_page(int clnt_write, char* serv_path);
void send_chunked(int clnt_write, char* chunked_data);
void send_chunked_end(int clnt_write);