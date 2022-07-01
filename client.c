#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char* retryConnection(char* message, int sockfd)
{
	char *response;
	sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	return response;
}

/* create a new account */
void registerUser(int sockfd) {
	char* message;
	char* response;

	char **data = (char**) calloc(2, sizeof(char*));
	data[0] = (char*) calloc(100, sizeof(char));
	printf("username=");
	scanf("%s", data[0]);

	data[1] = (char*) calloc(100, sizeof(char));
	printf("password=");
	scanf("%s", data[1]);
	printf("\n");

	/* create message and add cookies */
	message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register",
		"application/json", data, 2, NULL, 0, NULL, 0, 0);

	/* print message and send it to server */
	send_to_server(sockfd, message);
	puts(message);

	/* receive response from server */
	response = receive_from_server(sockfd);
	if (!strlen(response)) {
		/* retry */
		response = retryConnection(message, sockfd);
	}

	puts(response);
	printf("\n\n");

	/* verify the answer from server */
	if (!strncmp(response + 9, "201", 3)) {
		printf("\nThe account was successfully created.\n\n");
	} else if (!strncmp(response + 9, "400", 3)) {
		printf("\n!! ERROR !! The username %s is already taken!\n\n", data[0]);
	}	

	/* free memory */
	free(data[0]);
	free(data[1]);
	free(data);
	free(message);
}

/* Send the login message and add cookie in cookies' list */
char* loginCommand(int sockfd) {
	char* message;
	char* response;
	
	char **data = (char**) calloc(2, sizeof(char*));
	data[0] = (char*) calloc(DATALEN, sizeof(char));
	printf("username=");
	scanf("%s", data[0]);

	data[1] = (char*) calloc(DATALEN, sizeof(char));
	printf("password=");
	scanf("%s", data[1]);
	printf("\n");

	/* create message and add cookies */
	message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login",
		"application/json", data, 2, NULL, 0, NULL, 0, 0);

	/* print message and send it to server */
	send_to_server(sockfd, message);
	puts(message);

	/* receive response from server */
	response = receive_from_server(sockfd);
	if (!strlen(response)) {
		/* retry */
		response = retryConnection(message, sockfd);	
	}

	/* print response */
	puts(response);
	printf("\n\n");

	/* extract the cookie */
	char *cookie = strstr(response, "connect");
	if (cookie) {
		cookie = strtok(cookie, ";");
		printf("Loading.\n");
		sleep(1);
		printf("Loading..\n");
		sleep(1);
		printf("Loading...\n");
		sleep(1);
		printf("\nWelcome, %s!\n", data[0]);
		printf("\nSession cookie: %s.\n\n", cookie);
	}

	/* free memory */
	free(data[0]);
	free(data[1]);
	free(data);
	free(message);
	return cookie;
}

/* request access to library */
char* enter_libraryCommand (int sockfd, char** cookies, int isLoged) {
	char *token;
	char *response;
	char *message;
	
	/* create message */
	message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", 
										NULL , cookies, isLoged, NULL, 0);
	
	/* send the message to server */
	send_to_server(sockfd, message);
	
	/* receive response from server */
	response = receive_from_server(sockfd);
	if  (!strlen(response)) {
		/* retry */
		retryConnection(message, sockfd);
	}	

	/* print response */
	puts(response);

	/* check response */
	if (!strncmp(response + 9, "200", 3)) {
		printf("\nNow you have the access to the library.\n\n");
		token = (strstr(response, "{")) + 10;
		token = strtok(token, "\"");
	} else {
		printf("\n!! ERROR !! Failed to get access.\n\n");
		return NULL;
	}
	return token;
}

/* add book in library */
void addOneBook(int sockfd, char** tokens, int hasAccess) {
	char *message;
	char *response;
	char **data;

	/* alloc data for info */
	data = (char**)calloc(5, sizeof(char*));
	for(int i = 0; i < 5; i++) {
		data[i] = (char*)calloc(100,sizeof(char));
	}

	/* read data from stdin */
	char title[50], author[50], genre[20], publisher[20], page_count[5];  
	printf("title=");
	scanf("%s",title);
	memcpy(data[0], title, TITLELEN * sizeof(char));

	printf("author=");
	scanf("%s",author);
	memcpy(data[1], author, AUTHORLEN * sizeof(char));

	printf("genre=");
	scanf("%s",genre);
	memcpy(data[2], genre, GENRELEN * sizeof(char));

	printf("publisher=");
	scanf("%s",publisher);
	memcpy(data[3], publisher, PUBLISHERLEN * sizeof(char));
	
	printf("page_count=");
	scanf("%s",page_count);
	memcpy(data[4], page_count, PAGECOUNTLEN * sizeof(char));		

	/* check page_count validity */
	for(int i = 0; i < strlen(page_count); i++) {
		if (page_count[i] < '0' || page_count[i] > '9') {
			printf("\nPage_count invalid.\n\n");
			return;
		}
	}

	/* create message */
	message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books",
		"application/json", data, 5, NULL, 0, tokens, hasAccess, 1);
	
	/* send and print messages */
	send_to_server(sockfd, message);
	puts(message);
	
	/* receive response from server */
	response = receive_from_server(sockfd);
	if (!strlen(response)) {
		/* retry */
		retryConnection(message, sockfd);	
	}

	/* print response */
	puts(response);

	/* check response */
	if (strncmp(response + 9, "200", 3)) {
		printf("\n!! ERROR !! Couldn't add the book to the library.\n\n");
	} else {
		printf("\nThe book was added succesfully.\n\n");
	}

	/* free data */
    for(int i = 0; i < 5; i++)
        free(data[i]);
    free(data);
	free(response);
	free(message);
}

/* print all books */
void printBooks(int sockfd, char** tokens, int hasAccess) {
	char *message;
	char *response;

	/* create message */
	message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, NULL, 0, tokens, hasAccess);
	
	/* send message to server */
	send_to_server(sockfd, message);
	
	/* get response from the server */
	response = receive_from_server(sockfd);
	if (!strlen(response)) {
		/* retry */
		retryConnection(message, sockfd);	
	}	

	/* print message */
	puts(response);

	/* check response */
	if (strncmp(response + 9, "200", 3)) {
		printf("!! ERROR !! Can't show the books.\n\n");
	} else {
		printf("\nThese are all the books.\n\n");
	}

	/* free data */
	free(response);
	free(message);
}

/* print a book by its ID */
void printOneBook(int sockfd, char** tokens, int hasAccess) {
	char *message;
	char *response;

	/* read data from stdin */
	char *id = malloc(10 * sizeof(char));
	printf("id=");
	scanf("%s", id);
	printf("\n");

	/* check ID */
	for(int i = 0; i < strlen(id); i++) {
		if (id[i] < '0' || id[i] > '9') {
			printf("!! ERROR !! Invalid ID.\n\n");
			return;
		}
	}
	
	/* create url for book */
	char url[] = "/api/v1/tema/library/books/";
	strcat(url, id);
	
	/* create message */
	message = compute_get_request("34.241.4.235", url, NULL, NULL, 0, tokens, hasAccess);
	
	/* send and print message */
	send_to_server(sockfd, message);
	puts(message);
	
	/* receive response from server */
	response = receive_from_server(sockfd);
	if (!strlen(response)) {
		/* retry */
		retryConnection(message, sockfd);
	}	

	/* print response */
	puts(response);

	/* check response */
	if (strncmp(response + 9, "200", 3)) {
		printf("\n!! ERROR !! There are no books with the given ID.\n\n");
	} else {
		printf("\nThis is the wanted book.\n\n");
	}

	/* free data */
	free(message);
	free(response);
}

/* delete a book from library */
void deleteOneBook(int sockfd, char **tokens, int hasAccess) {
	char *message;
	char *response;

	/* read data from stdin */
	char *id = malloc(10 * sizeof(char));
	printf("id=");
	scanf("%s",id);

	/* check ID */
	for(int i = 0; i < strlen(id); i++) {
		if (id[i] < '0' || id[i] > '9') {
			printf("\n!! ERROR !! Id invalid.\n\n");
			return;
		}
	}
	
	/* create url for book */
	char url[] = "/api/v1/tema/library/books/";
	strcat(url, id);
	
	/* create message */
	message = compute_delete_request("34.241.4.235", url, NULL, tokens, hasAccess);
	
	/* send and print message */
	send_to_server(sockfd, message);
	puts(message);
	
	/* get response from server */
	response = receive_from_server(sockfd);
	if (!strlen(response)) {
		/* retry */
		retryConnection(message, sockfd);	
	}

	/* print response */
	puts(response);

	/* check response */
	if (strncmp(response + 9, "200", 3)) {
		printf("\n!! ERROR !! Couldn't delete the book from the library.\n\n");
	} else {
		printf("\nThe book was deleted from the library.\n\n");
	}	

	/* free data */
	free(response);
	free(message);
}

/* logout */
void logoutCommand(int sockfd, char **cookies, int isLoged) {
	char *response;
	char *message;
	
	/* create message */
	message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", NULL,
		cookies, isLoged, NULL, 0);
	
	/* send and print message */
	send_to_server(sockfd, message);
	puts(message);
	
	/* receive response */
	response = receive_from_server(sockfd);
	if (!strlen(response)) {
		/* retry */
		retryConnection(message, sockfd);	
	}	

	/* print response */
	puts(response);

	/* check response */
	if (strncmp(response + 9, "200", 3)) {
		printf("\n!! ERROR !! Couldn't log out.\n\n");
	} else {
		printf("\nYou logged out.\n\n");
	}

	/* free data */
	free(response);
	free(message);
}

int main(int argc, char *argv[])
{
	char command[20];
    int sockfd;
	int hasAccess = 0;
	int isLoged = 0;
	char **tokens;
	char **cookies;

	/* tokens and cookie lists */
	tokens = (char**)calloc(10, sizeof(char*));
	cookies = (char**)calloc(10, sizeof(char*));

	for (int i = 0; i < 5; i++) {
		tokens[i] = (char*)calloc(LENGTH, sizeof(char));
	}

	for (int i = 0; i < 5; i++) {
		cookies[i] = (char*)calloc(LENGTH, sizeof(char));
	}

	while(1) {
		/* open socket */
		sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
		fgets(command, 19, stdin);

		/* check commands */
		if (!strncmp(command, "register", 8)) {
			registerUser(sockfd);
		} else if (!strncmp(command, "login", 5)) {
			if (isLoged) {
				printf("\n!! ERROR !! An user is already connected.\n");
				printf("\n");
			}
			else {
				char* cookie = loginCommand(sockfd);
				if (cookie) {
					memcpy(cookies[0], cookie, LENGTH * sizeof(char));
					isLoged = 1;
				} else {
					printf("\n!! ERROR !! Log in failed\n\n");
					printf("\n");
				}
			}
		} else if (!strncmp(command, "enter_library", 13)) {
			if(!isLoged) {
				printf("\n!! ERROR !! Please, log in first.\n");
				printf("\n");
				continue;
			}
			else {
				char* token = enter_libraryCommand(sockfd, cookies, isLoged);
				if (token) {
					memcpy(tokens[0], token, LENGTH * sizeof(char));
					hasAccess = 1;
				}
			}
		} else if (!strncmp(command, "get_books", 9)) {
			if (!hasAccess) {
				printf("\n!! ERROR !! You don't have acces to the library.\n\n");
			}
			else {
				printBooks(sockfd, tokens, hasAccess);
			}		
		} else if (!strncmp(command, "get_book", 8)) {
			if (!hasAccess) {
				printf("\n!! ERROR !! You don't have acces to the library.\n\n");
			}
			else {
				printOneBook(sockfd, tokens, hasAccess);
			}
		} else if (!strncmp(command, "add_book", 8)) {
			if (!hasAccess) {
				printf("\n!! ERROR !! You don't have acces to the library.\n\n");
			}
			else {
				addOneBook(sockfd, tokens, hasAccess);
			}			 
		} else if (!strncmp(command, "delete_book", 11)) {
			if (!hasAccess) {
				printf("\n!! ERROR !! You don't have acces to the library.\n\n");
			}
			else {
				deleteOneBook(sockfd, tokens, hasAccess);
			}			
		} else if (!strncmp(command, "logout", 6)) {
			if (!isLoged) {
				printf("\n!! ERROR !! You have to log in first.\n\n");
			}
			else {
				hasAccess = 0;
				logoutCommand(sockfd, cookies, isLoged);
				isLoged = 0;
			}
		} else if (!strncmp(command, "exit", 4)) {
			printf("\nGood bye!\n");
			break;
		}
	}

	/* close socket */
	close(sockfd);
    return 0;
}
