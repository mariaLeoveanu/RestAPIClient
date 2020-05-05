char* extract_info_response(char *from, char* until, char* body);
char* restore_connection(int* sockfd, char* message, char* response);
void register_command(int* sockfd, char* message, char* response);
char* login_command(int* sockfd, char* message, char* response);
char* enter_library_command(int* sockfd, char* message, char* response, char* cookies);
void get_books_command(int* sockfd, char* message, char* response, char* token);
void add_book_command(int* sockfd, char* message, char* response, char* token);
void get_book_command(int* sockfd, char* message, char* response, char* token);
void delete_book_command(int* sockfd, char* message, char* response, char* token);
void logout_command(int* sockfd, char* message, char* response, char* cookies);


