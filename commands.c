#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#define PORT 8080
#define COMMAND_SIZE 101
#define MESSAGE_SIZE 10000


// helper function that returns content from HTTP response 
// <AAA>Target<BBB> => extracts Target between <AAA> and <BBB>
char* extract_info_response(char *from, char* until, char* body){
	char *start, *end;
	char *result;
	if((start = strstr(body, from))){
		start += strlen(from);
		if((end = strstr(start, until))){
			result =(char*)malloc(end - start + 1);
			memcpy(result, start, end - start);
			result[end -start] = '\0';
		}
	}
	
	return result;
}

// function that restores connection
char* restore_connection(int* sockfd, char* message, char* response){
	*sockfd = open_connection("3.8.116.10", PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    return response;
}


void register_command(int* sockfd, char* message, char* response){

	JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char* login_data[1];
    char* cont_type = NULL;
    char* username;
    char* password;
    	
    username = malloc(sizeof(char) * COMMAND_SIZE);
    password = malloc(sizeof(char) * COMMAND_SIZE);
    	

    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    // add to login data string json
    login_data[0] = json_serialize_to_string_pretty(root_value);
    cont_type = "application/json";
    		
    message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/register", cont_type, login_data, 1, NULL, 0, NULL, 0);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    // if the client doesn't get a response => tries to restore the connection
    if(strlen(response) == 0){
    	response = restore_connection(sockfd, message, response);
    }
   
    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
}

char* login_command(int* sockfd, char* message, char* response){

	JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char* cont_type = "application/json";
    char* user_info[1];
    char* username;
    char* password;

    username = malloc(sizeof(char) * COMMAND_SIZE);
    password = malloc(sizeof(char) * COMMAND_SIZE);

    // read user input
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    // create json body
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    user_info[0] = json_serialize_to_string_pretty(root_value);

    message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/login", cont_type, user_info, 1, NULL, 0, NULL, 0);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    while(strlen(response) == 0){
    	// while there is no response from server, restablish connection and try again
    	response = restore_connection(sockfd, message, response);
    }
    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
    return response;
}

char* enter_library_command(int* sockfd, char* message, char* response, char* cookies){

	message = compute_get_request("3.8.116.10", "/api/v1/tema/library/access", NULL, &cookies, 1, NULL, 0);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    while(strlen(response) == 0){
     	// while there is no response from server, restablish connection and try again
     	response = restore_connection(sockfd, message, response);
    }
    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
    // token is extracted from this response in client.c
    return response;
}

void get_books_command(int* sockfd, char* message, char* response, char* token){
	message = compute_get_request("3.8.116.10", "/api/v1/tema/library/books", NULL, NULL, 0, &token, 1);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    while(strlen(response) == 0){
    	response = restore_connection(sockfd, message, response);
    }
    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
}

void add_book_command(int* sockfd, char* message, char* response, char* token){
	char *title, *author, *genre, *publisher, *page_count_string, *temp;
    char* cont_type = "application/json";
    char* book_info[1];
    int page_count;

    title = malloc(sizeof(char) * COMMAND_SIZE);
    author = malloc(sizeof(char) * COMMAND_SIZE);
    genre = malloc(sizeof(char) * COMMAND_SIZE);
    publisher = malloc(sizeof(char) * COMMAND_SIZE);
    page_count_string = malloc(sizeof(char) * COMMAND_SIZE);
    temp = malloc(sizeof(char) * COMMAND_SIZE);

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    printf("title=");
    scanf("%c",temp);
    scanf("%[^\n]", title);

    printf("author=");
    scanf("%c",temp);
    scanf("%[^\n]", author);

    printf("genre=");
    scanf("%c",temp);
    scanf("%[^\n]", genre);

    printf("publisher=");
    scanf("%c",temp);
    scanf("%[^\n]", publisher);

    printf("page_count=");
    scanf("%s", page_count_string);
    // if input for number of pages is not int
    // display warning and input prompt again
    while(atoi(page_count_string) == 0){
    	printf("\t\t--------- WARNING ----------\n");
    	printf("Not a valid input. Digits only!\n");
    	printf("page_count=");
    	scanf("%s", page_count_string);
    }
    page_count = atoi(page_count_string);

    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_number(root_object, "page_count", page_count);
    json_object_set_string(root_object, "publisher", publisher);

    book_info[0] = json_serialize_to_string_pretty(root_value);

    message = compute_post_request("3.8.116.10", "/api/v1/tema/library/books", cont_type, book_info, 1, NULL, 0, &token, 1);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    while(strlen(response) == 0){
    	response = restore_connection(sockfd, message, response);
    }
    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
}

void get_book_command(int* sockfd, char* message, char* response, char* token){
	char* id = malloc(sizeof(char) * COMMAND_SIZE);
    char* final_path = malloc(sizeof(char) * COMMAND_SIZE);
    char* path = "/api/v1/tema/library/books/";
    char* int_as_string = malloc(sizeof(char) * COMMAND_SIZE);

	printf("id=");
	scanf("%s", id);

	// check for a valid id(only digits)
	while(atoi(id) == 0){
		printf("\t\t--------- WARNING ----------\n");
   		printf("Not a valid input. Digits only!\n");
	    printf("id=");
    	scanf("%s", id);
    }
    sprintf(int_as_string, "%d", atoi(id));
    		
    strcpy(final_path, path);
    strcat(final_path, int_as_string);
   
    message = compute_get_request("3.8.116.10", final_path, NULL, NULL, 0, &token, 1);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    while(strlen(response) == 0){
    	response = restore_connection(sockfd, message, response);
    }

    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
}

void delete_book_command(int* sockfd, char* message, char* response, char* token){
	char* id = malloc(sizeof(char) * COMMAND_SIZE);
    char* final_path = malloc(sizeof(char) * COMMAND_SIZE);
    char* path = "/api/v1/tema/library/books/";
    char* int_as_string = malloc(sizeof(char) * COMMAND_SIZE);

    printf("id=");
    scanf("%s", id);
    while(atoi(id) == 0){
    	printf("\t\t--------- WARNING ----------\n");
    	printf("Not a valid input. Digits only!\n");
    	printf("id=");
    	scanf("%s", id);
    }
    sprintf(int_as_string, "%d", atoi(id));
    
    // create path for delete request with input from user	
    strcpy(final_path, path);
    strcat(final_path, int_as_string);

    message = compute_delete_request("3.8.116.10", final_path, NULL, NULL, 0, NULL, 0, &token, 1);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    while(strlen(response) == 0){
    	response = restore_connection(sockfd, message, response);
    }
    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
}

void logout_command(int* sockfd, char* message, char* response, char* cookies){
	
	message = compute_get_request("3.8.116.10", "/api/v1/tema/auth/logout", NULL, &cookies, 1, NULL, 0);
    printf("\t\t---------- SENT MESSAGE ----------\n");
    printf("%s\n", message);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    while(strlen(response) == 0){
    	response = restore_connection(sockfd, message, response);
    }
    printf("\t\t---------- RECEIVED MESSAGE ----------\n");
    printf("%s\n", response);
}