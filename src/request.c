#include "request.h"
#include "string.h"
#include "log.h"
#include "http_parser.h"

int request_parse(Request* request, char* data, unsigned int length);

int url_callback(http_parser* parser, const char *at, size_t length) {
	strncpy(((Request*)parser->data)->path, at, length);
	return 0;
}

int header_field_callback(http_parser* parser, const char *at, size_t length) {
	Request* request = parser->data;
	memset(request->tmp_field, 0, REQUEST_MAX_HEADER_KEY);
	strncpy(request->tmp_field, at, length);
	return 0;
}

int header_value_callback(http_parser* parser, const char *at, size_t length) {
	Request* request = parser->data;

	Request_Header* header = malloc_c(sizeof(Request_Header));
	if (header == NULL) {
		Log_Message("Out of memory");
		return 1;
	}

	strncpy(header->key, request->tmp_field, REQUEST_MAX_HEADER_KEY);
	strncpy(header->value, at, length);

	request->headers[request->headers_count++] = header;
	return 0;
}


int on_body_callback(http_parser* parser, const char *at, size_t length) {
	Request* request = parser->data;
	return 0;
}


int request_parse(Request* request, char* data, unsigned int length){
	 http_parser parser;
	 parser.data = request;
	 http_parser_init(&parser, HTTP_REQUEST);
	 http_parser_settings settings;
	 http_parser_settings_init(&settings);
	 settings.on_url = url_callback;
	 settings.on_header_field = header_field_callback;
	 settings.on_header_value = header_value_callback;
	 //settings.on_body = on_body_callback;

	 http_parser_execute(&parser, &settings, data, length);

	 request->type = GET;
	 if (parser.method == 3 ){
		 request->type = POST;
	 }

	 return 0;
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
