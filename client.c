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
#include "commands.h"

int main(int argc, char *argv[])
{
    char* message;
    char* response;
    char* command;
    char* cookies;
    char* token;
    int sockfd, logged = 0, recv_token = 0;
    command = malloc(sizeof(char) * COMMAND_SIZE);
    message = malloc(sizeof(char) * MESSAGE_SIZE);
   

    sockfd = open_connection("3.8.116.10", PORT, AF_INET, SOCK_STREAM, 0);
  
    while(1){
    	scanf("%s", command);
    	if(strcmp(command, "register") == 0){
    		register_command(&sockfd, message, response);
    	} else if(strcmp(command, "login") == 0){
    			if(logged == 0){
    				// logged = 0 => no user is logged and the user can login 
    				char* start_pattern = "Set-Cookie: ";
    				char* end_pattern = ";";
    				response = login_command(&sockfd, message, response);
     				cookies = extract_info_response(start_pattern, end_pattern, response);
     				if(strstr(response, "HTTP/1.1 200 OK") != NULL){
     					// set logged only if the login ended successfully
     				 	logged = 1;
     				} else {
     				 	printf("\t\t--------- WARNING ----------\n");
    					printf("Incorrect username and password!\n");
     				}
     				
    			} else {
    				// if a user is already logged in, display warning message 
    				// the client does not compute + send login request in this case
    				printf("\t\t--------- WARNING ----------\n");
    				printf("You are already logged in! Log out before switching accounts.\n");
    			}
	
    	} else if(strcmp(command, "enter_library") == 0){
    		if(logged == 0 || strlen(cookies) == 0){
    			printf("\t\t--------- WARNING ----------\n");
    			printf("You are not logged in!\n");
    		} else {
    			char* start_pattern = "{\"token\":\"";
     			char* end_pattern = "\"}";
     			response = enter_library_command(&sockfd, message, response, cookies);
     			token = extract_info_response(start_pattern, end_pattern, response);
     			// recv_token is set when token is extracted & reset when user logs out
     			recv_token = 1;
    		}
    		
    	} else if(strcmp(command, "get_books") == 0){
    		if(recv_token == 0){
    			printf("\t\t--------- WARNING ----------\n");
    			printf("You dont't have access to the library!\n");
    		} else {
    			get_books_command(&sockfd, message, response, token); 
    		}
    	} else if(strcmp(command, "add_book") == 0){
    		if(recv_token == 0){
    			printf("\t\t--------- WARNING ----------\n");
    			printf("You dont't have access to the library!\n");
    		} else {
    			add_book_command(&sockfd, message, response, token);
    		}
    	} else if(strcmp(command, "get_book") == 0){
    		if(recv_token == 0){
    			printf("\t\t--------- WARNING ----------\n");
    			printf("You dont't have access to the library!\n");
    		} else {
    			get_book_command(&sockfd, message, response, token);
    		}
    	} else if(strcmp(command, "delete_book") == 0){
    		if(recv_token == 0){
    			printf("\t\t--------- WARNING ----------\n");
    			printf("You dont't have access to the library!\n");
    		} else {
    			delete_book_command(&sockfd, message, response, token);
    		}
    	} else if(strcmp(command, "logout") == 0){
    		if(logged == 0){
    			printf("\t\t--------- WARNING ----------\n");
    			printf("You are not logged in!\n");
    		} else {
    			
    			logout_command(&sockfd, message, response, cookies);
    			// reset flags
    			logged = 0;
    			recv_token = 0;
    		}
    	}else if(strcmp(command, "exit") == 0){
    		close_connection(sockfd);
    		return 0;
    	} else {
    		printf("\t\t--------- WARNING ----------\n");
    		printf("Not a valid command! Try again!\n");
    	}
    }
    
    return 0;
}
