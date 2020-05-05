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
    char *message;
    char *response;
    int sockfd;
    char *command;
    char* cookies;
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
    		
    		message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/register", cont_type, login_data, 1, NULL, 0);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		if(strlen(response) == 0){
    		 	message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/register", cont_type, login_data, 1, NULL, 0);
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

    		message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/login", cont_type, user_info, 1, NULL, 0);
    		printf("%s\n", message);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
    		while(strlen(response) == 0){

    			// while there is no response from server, restablish connection and try again
    			message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/login", cont_type, user_info, 1, NULL, 0);
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

     		message = compute_get_request("3.8.116.10", "/api/v1/tema/library/access", NULL, &cookies, 1);
     		printf("%s\n", message);
     		send_to_server(sockfd, message);
     		response = receive_from_server(sockfd);
     		while(strlen(response) == 0){

     			// while there is no response from server, restablish connection and try again
     			message = compute_get_request("3.8.116.10", "/api/v1/tema/library/access", NULL, &cookies, 1);
     			printf("%s\n", message);
     			sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
    			send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
     		}
     		printf("%s\n", response);
     		
    	}
    }
    

    // Ex 1.2: POST dummy and print response from main server
    // char* data[1];
    // data[0] = "mesaj=dummy";
    // char* cont_type = "application/x-www-form-urlencoded";
    // message = compute_post_request("3.8.116.10", "/api/v1/dummy", cont_type, data, 1, NULL, 0);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // printf("%s\n", response);

    // Ex 2: Login into main server
    // char* login_data[2];
    // login_data[0] = "username=student";
    // login_data[1] = "password=student";
    // cont_type = "application/x-www-form-urlencoded";
    // message = compute_post_request("3.8.116.10", "/api/v1/auth/login", cont_type, login_data, 2, NULL, 0);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // printf("%s\n", response);

    // Ex 3: GET weather key from main server
     

    // Ex 4: GET weather data from OpenWeather API
    // int weather_socket = open_connection("37.139.1.159", 80, AF_INET, SOCK_STREAM, 0);
    // char* parameter = "lat=18&lon=34&appid=b912dd495585fbf756dc6d8f415a7649";
    // message = compute_get_request("37.139.1.159", "/data/2.5/weather", parameter, NULL, 0);
    // puts(message);
    // send_to_server(weather_socket, message);
    // response = receive_from_server(weather_socket);
    // printf("%s\n",response);

    // Ex 5: POST weather data for verification to main server
    // cont_type = "appliction/json";
    // char* content[1];
    // content[0] = basic_extract_json_response(response);
    // message = compute_post_request("3.8.116.10", "/api/v1/weather/18/34", cont_type, content, 1, cookies, 1);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // printf("%s\n",response);

    // Ex 6: Logout from main server
    // message = compute_get_request("3.8.116.10", "/api/v1/auth/logout", NULL, NULL, 0);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // printf("%s\n",response);

    // BONUS: make the main server return "Already logged in!"

    // free the allocated data at the end!

    return 0;
}
