#include "request.h"
#include "string.h"
#include "log.h"
#include "http_parser.h"
#include "picohttpparser.h"
#include <stdlib.h>
#include <ctype.h>


int request_parse(Request* request, char* data, unsigned int length){
	char *method, *path;
	int pret, minor_version;
	struct phr_header headers[100];
	size_t method_len, path_len, num_headers;

	num_headers = sizeof(headers) / sizeof(headers[0]);

	pret = phr_parse_request(data, length, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, 0);
	if (pret == -1){
		Log_Message("Request ParseError");
		goto fail;
	}
	if (length - pret != 0){
		request_parse_payload(request, &data[pret]);
	}

	strncpy(request->path, path, path_len);

	for (unsigned int i=0; i<num_headers; i++){
		if (headers[i].name_len > REQUEST_MAX_HEADER_KEY || headers[i].value_len > REQUEST_MAX_HEADER_VALUE ){
			Log_Message("header size too big");
			continue;
		}

		Request_Header* header = malloc_c(sizeof(Request_Header));
		if (header == NULL) {
			Log_Message("Out of memory");
			goto fail;
		}
		strncpy(header->key, headers[i].name, headers[i].name_len);
		strncpy(header->value, headers[i].value, headers[i].value_len);

		if ( strncmp(header->key, "Content-Length", 14) == 0 ){
			request->content_length = atoi(header->value);
		}

		request->headers[request->headers_count++] = header;
	}

	request->type = GET;
	if (strncmp(method, "POST", method_len) == 0){
		request->type = POST;
	}
	return 0;

	fail:
		Log_Message("Request parser error. Try to free resources");
		while( (request->headers_count--) >= 0 ){
			free_c(request->headers[request->headers_count]);
		}

		return 1;
}

void request_free(Request* request){
	for(int i=0;i<request->headers_count;i++){
		free_c(request->headers[i]);
	}

	for(int i=0;i<request->data_count;i++){
		free_c(request->data[i]);
	}
}


int request_get_post_field(Request* request, char* field_name, char* field_value){
	for(int i=0; i< request->data_count;i++){
		Request_Data* data= request->data[i];
		if( strcmp(data->key, field_name) == 0 ){
			strncpy(field_value, data->value, REQUEST_DATA_VALUE_LEN);
			return 0;
		}
	}

	return 1;
}

void request_parse_payload(Request* request, char* data){
	char *end_str;
	char *token = strtok_r(data, "&", &end_str);

	while(token != NULL){
		char decoded[REQUEST_DATA_VALUE_LEN] ={0};
	    char *end_token;

	    char* key = strtok_r(token, "=", &end_token);
	    char* value = strtok_r(NULL, "=", &end_token);
		urldecode2(decoded, value);

		Request_Data* data_entry = (Request_Data*)malloc_c(sizeof(Request_Data));
		if(data_entry == NULL){
			Log_Message("Out of memory");
			return;
		}

		strncpy(data_entry->key, key, REQUEST_DATA_KEY_LEN);
		strncpy(data_entry->value, decoded, REQUEST_DATA_VALUE_LEN);
		request->data[request->data_count++] = data_entry;

		token = strtok_r(NULL, "&", &end_str);
	}
}

void urldecode2(char *dst, const char *src){
	char a, b;
	while (*src) {
			if ((*src == '%') &&
				((a = src[1]) && (b = src[2])) &&
				(isxdigit(a) && isxdigit(b))) {
					if (a >= 'a')
							a -= 'a'-'A';
					if (a >= 'A')
							a -= ('A' - 10);
					else
							a -= '0';
					if (b >= 'a')
							b -= 'a'-'A';
					if (b >= 'A')
							b -= ('A' - 10);
					else
							b -= '0';
					*dst++ = 16*a+b;
					src+=3;
			} else if (*src == '+') {
					*dst++ = ' ';
					src++;
			} else {
					*dst++ = *src++;
			}
	}
	*dst++ = '\0';
}
