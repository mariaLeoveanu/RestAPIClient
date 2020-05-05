#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

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

char* restore_connection(int* sockfd, char* message, char* response){
	*sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    send_to_server(*sockfd, message);
    response = receive_from_server(*sockfd);
    return response;
}

int main(int argc, char *argv[])
{
    char* message;
    char* response;
    char* command;
    char* cookies;
    char* token;
    int sockfd;
    command = malloc(sizeof(char) * 101);
    sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
        
    
    while(1){
    	scanf("%s", command);

    	if(strcmp(command, "register") == 0){

    		JSON_Value *root_value = json_value_init_object();
    		JSON_Object *root_object = json_value_get_object(root_value);
    		char* login_data[1];
    		char* cont_type = NULL;
    		char* username;
    		char* password;
    	
    		username = malloc(sizeof(char) * 101);
    		password = malloc(sizeof(char) * 101);
    	

    		printf("username:");
    		scanf("%s", username);
    		printf("password:");
    		scanf("%s", password);

    		json_object_set_string(root_object, "username", username);
    		json_object_set_string(root_object, "password", password);

    		login_data[0] = json_serialize_to_string_pretty(root_value);
    		cont_type = "application/json";
    		
    		message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/register", cont_type, login_data, 1, NULL, 0, NULL, 0);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		if(strlen(response) == 0){
    		 	message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/register", cont_type, login_data, 1, NULL, 0, NULL, 0);
    		 	printf("%s\n", message);
    		 	sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    		 	send_to_server(sockfd, message);
    		 	response = receive_from_server(sockfd);
    		 }
    		printf("%s\n", response);
    		
    	}

    	if(strcmp(command, "login") == 0){
			
    		JSON_Value *root_value = json_value_init_object();
    		JSON_Object *root_object = json_value_get_object(root_value);
    		char* cont_type = "application/json";
    		char* user_info[1];
    		char* username;
    		char* password;

    		username = malloc(sizeof(char) * 101);
    		password = malloc(sizeof(char) * 101);


    		// read user input
    		printf("username:");
    		scanf("%s", username);
    		printf("password:");
    	    scanf("%s", password);

    		// create json body
    		json_object_set_string(root_object, "username", username);
    		json_object_set_string(root_object, "password", password);
    		user_info[0] = json_serialize_to_string_pretty(root_value);

    		message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/login", cont_type, user_info, 1, NULL, 0, NULL, 0);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		while(strlen(response) == 0){

    			// while there is no response from server, restablish connection and try again
    			message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/login", cont_type, user_info, 1, NULL, 0, NULL, 0);
     			printf("%s\n", message);
     			sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    			send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
     		}
    		printf("%s\n", response);
    		
    		// extact cookie
    		char* start_pattern = "Set-Cookie: ";
    		char* end_pattern = ";";
     		cookies = extract_info_response(start_pattern, end_pattern, response);  
     		printf("Extracted cookie:%s\n", cookies);  		
    	}
    	if(strcmp(command, "enter_library") == 0){

     		message = compute_get_request("3.8.116.10", "/api/v1/tema/library/access", NULL, &cookies, 1, NULL, 0);
     		printf("%s\n", message);
     		send_to_server(sockfd, message);
     		response = receive_from_server(sockfd);
     		while(strlen(response) == 0){

     			// while there is no response from server, restablish connection and try again
     			message = compute_get_request("3.8.116.10", "/api/v1/tema/library/access", NULL, &cookies, 1, NULL, 0);
     			printf("%s\n", message);
     			sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    			send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
     		}
     		printf("%s\n", response);

     		// extract token
     		char* start_pattern = "{\"token\":\"";
     		char* end_pattern = "\"}";
     		token = extract_info_response(start_pattern, end_pattern, response);
     		printf("Extracted token:%s\n", token);
     		
    	}

    	if(strcmp(command, "get_books") == 0){
    		
    		message = compute_get_request("3.8.116.10", "/api/v1/tema/library/books", NULL, NULL, 0, &token, 1);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		while(strlen(response) == 0){
    			message = compute_get_request("3.8.116.10", "/api/v1/tema/library/books", NULL, NULL, 0, &token, 1);
    			printf("%s\n", message);
    			sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    			send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
    		}
    		printf("%s\n", response);
    	}

    	if(strcmp(command, "add_book") == 0){

    		char *title, *author, *genre, *publisher;
    		char* cont_type = "application/json";
    		char* book_info[1];
    		int page_count;

    		title = malloc(sizeof(char) * 101);
    		author = malloc(sizeof(char) * 101);
    		genre = malloc(sizeof(char) * 101);
    		publisher = malloc(sizeof(char) * 101);

    		JSON_Value *root_value = json_value_init_object();
    		JSON_Object *root_object = json_value_get_object(root_value);

    		printf("title:");
    		scanf("%s", title);
    		printf("author:");
    	    scanf("%s", author);
    	    printf("genre:");
    	    scanf("%s", genre);
    	    printf("page_count:");
    	    scanf("%d", &page_count);
    	    printf("publisher:");
    	    scanf("%s", publisher);

    	    json_object_set_string(root_object, "title", title);
    		json_object_set_string(root_object, "author", author);
    		json_object_set_string(root_object, "genre", genre);
    		json_object_set_number(root_object, "page_count", page_count);
    		json_object_set_string(root_object, "publisher", publisher);

    		book_info[0] = json_serialize_to_string_pretty(root_value);

    		message = compute_post_request("3.8.116.10", "/api/v1/tema/library/books", cont_type, book_info, 1, NULL, 0, &token, 1);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		while(strlen(response) == 0){
    			message = compute_post_request("3.8.116.10", "/api/v1/tema/library/books", cont_type, book_info, 1, NULL, 0, &token, 1);
    			printf("%s\n", message);
    			sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    			send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
    		}

    		printf("%s\n", response);

    	}

    	if(strcmp(command, "get_book") == 0){
    		
    		char* id = malloc(sizeof(char) * 101);
    		char* final_path = malloc(sizeof(char) * 101);
    		char* path = "/api/v1/tema/library/books/";

    		printf("id:");
    		scanf("%s", id);
    		
    		strcpy(final_path, path);
    		strcat(final_path, id);
    		printf("%s\n", final_path);

    		message = compute_get_request("3.8.116.10", final_path, NULL, NULL, 0, &token, 1);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		while(strlen(response) == 0){
				message = compute_get_request("3.8.116.10", final_path, NULL, NULL, 0, &token, 1);    			
				printf("%s\n", message);
    			sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    			send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
    		}

    		printf("%s\n", response);

    	}
    	if(strcmp(command, "delete_book") == 0){
    		
    		char* id = malloc(sizeof(char) * 101);
    		char* final_path = malloc(sizeof(char) * 101);
    		char* path = "/api/v1/tema/library/books/";

    		printf("id:");
    		scanf("%s", id);
    		
    		strcpy(final_path, path);
    		strcat(final_path, id);
    		printf("%s\n", final_path);

    		message = compute_delete_request("3.8.116.10", final_path, NULL, NULL, 0, NULL, 0, &token, 1);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		while(strlen(response) == 0){
				message = compute_delete_request("3.8.116.10", final_path, NULL, NULL, 0, NULL, 0, &token, 1);    			
				printf("%s\n", message);
    			sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    			send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
    		}

    		printf("%s\n", response);

    	}

    }
    
    return 0;
}
