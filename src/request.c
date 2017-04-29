#include "request.h"
#include "string.h"
#include "log.h"
#include "http_parser.h"
#include "picohttpparser.h"

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
