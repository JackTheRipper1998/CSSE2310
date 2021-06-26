void communication_error();

void client_kicked();

void int_length(int* nameTakenNum, int* numLength);

int check_contain_colon(char* buffer);

int read_stdin_dynamic(char** buffer);

void read_file_dynamic(char** buffer, FILE* fp);

void handle_left_cmd(char* buffer);

int read_fdin_dynamic(char** buffer, FILE* readFD);

void free_malloc_char_pointer(char** buffer);

void split_buffer(char* buffer, char** content);
