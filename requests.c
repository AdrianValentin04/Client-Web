#include <stdlib.h>     
#include <stdio.h>
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.c"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count,
							char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);
    sprintf(line, "HOST: %s", host);
    compute_message(message, line);

    if (cookies_count != 0) {
        for(int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s",cookies[i]);
            compute_message(message, line);
        }
    }

    if (tokens != NULL) {
        for(int i = 0; i < tokens_count; i++) {
            sprintf(line, "Authorization: Bearer %s",tokens[i]);
            compute_message(message, line);
        }
    }

    compute_message(message, "");
	free(line);
    return message;
}

void parseFunction(int add_book, char** body_data, JSON_Value *root_value) {
	JSON_Object *root_object = json_value_get_object(root_value);

    if (add_book == 1) {
        json_object_set_string(root_object, "title", body_data[0]);
        json_object_set_string(root_object, "author", body_data[1]); 
        json_object_set_string(root_object, "genre", body_data[2]);
        json_object_set_string(root_object, "publisher", body_data[3]); 
        json_object_set_number(root_object, "page_count", atoi(body_data[4])); 
    } else {
		json_object_set_string(root_object, "username", body_data[0]);
        json_object_set_string(root_object, "password", body_data[1]);
	}
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count,
							char **tokens, int tokens_count, int add_book)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "HOST: %s", host);
    compute_message(message, line);

    if (tokens != NULL) {
        for(int i = 0; i < tokens_count; i++) {
            sprintf(line, "Authorization: Bearer %s",tokens[i]);
            compute_message(message, line);
        }
    }

    sprintf(line, "Content-Type: %s",content_type);
    compute_message(message, line);

	JSON_Value *root_value = json_value_init_object();
    parseFunction(add_book, body_data, root_value);

    char *serialized_string = json_serialize_to_string_pretty(root_value); 

    sprintf(line, "Content-Length: %ld", strlen(serialized_string));
    compute_message(message, line);

    if (cookies_count != 0) {
        for(int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s",cookies[i]);
            compute_message(message, line);
        }
    }

    compute_message(message, "");
    memset(line, 0, LINELEN);
    compute_message(message, serialized_string);

    json_value_free(root_value);
	json_free_serialized_string(serialized_string);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url,char *query_params, char** tokens, int tokens_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    /* write the method name, the URL and protocol */
    sprintf(line, "DELETE %s HTTP/1.1", url);
    
    compute_message(message, line);

    /* write the host */
    sprintf(line, "HOST: %s", host);
    compute_message(message, line);
   
    /* write the authorisation */
    if (tokens != NULL) {
        for(int i = 0; i < tokens_count; i++) {
            sprintf(line, "Authorization: Bearer %s",tokens[i]);
            compute_message(message, line);
        }
    }

    /* blank line */
    compute_message(message, "");
    free(line);
    return message;	
}