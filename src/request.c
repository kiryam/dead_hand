#include "request.h"
#include "string.h"
#include "log.h"
#include "http_parser.h"


int request_parse(Request* request, char* data, unsigned int length);
/*
int request_init(Request* request, char* data, unsigned int length){
	if (length > 1024) {
		return 1;
	} // TODO

	if( request_parse(request, data, length) != 0){
		Log_Message("Parse header error");
		//free_c(request);
		return 1;
	}

	return 0;
}*/

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


	// free_c(parser);

	 return 0;
}

/*
int request_parse(Request* request, char* raw_header){
	char* ptr = raw_header;
	int path_cur=0;

	// Read message type
	if( strncmp("GET", ptr, 3) == 0 ){
		ptr = &ptr[4];
		request->type = GET;
	}

	if( strncmp("POST", ptr, 4) == 0 ){
		ptr = &ptr[5];
		request->type = POST;
	}

	// Read path
	while( ptr[path_cur] != ' ' && path_cur < REQUEST_MAX_PATH_LEN  ){
		path_cur++;
	}

	if( path_cur >= REQUEST_MAX_PATH_LEN ){
		Log_Message("Failed to read path");
		return 1;
	} else {
		strncpy(request->path, ptr, path_cur);
		request->path[path_cur] = '\0';
		ptr = &ptr[path_cur+1];
		path_cur = 0;
	}

	// Read protocol version
	while( ptr[path_cur] != '\n' && path_cur < REQUEST_MAX_PATH_LEN  ){
		path_cur++;
	}

	if( path_cur >= REQUEST_MAX_PATH_LEN ){
		Log_Message("Failed to read proto");
		return 1;
	} else {
		strncpy(request->proto, ptr, path_cur-1);
		request->proto[path_cur-1] = '\0'; // /r/n
		ptr = &ptr[path_cur+1];
		path_cur = 0;
	}

	// Read headers
	int header_parsed=0;
	int header_parsed_done=0;
	while (header_parsed < REQUEST_MAX_HEADER_COUNT && header_parsed_done == 0){
		while( ptr[path_cur] != '\n' && path_cur < REQUEST_MAX_HEADER_RAW_KV  ){
			path_cur++;
		}

		if( path_cur >= REQUEST_MAX_HEADER_RAW_KV ){
			Log_Message("Failed to read on header");
			return 1;
		} else {
			Request_Header* header = malloc_c(sizeof(Request_Header));

			char *token;
			char line[REQUEST_MAX_HEADER_RAW_KV]={0};
			char *search = ": ";
			strncpy(line, ptr, path_cur-1);

			int key_len = 0;
			while(key_len <= REQUEST_MAX_HEADER_KEY && line[++key_len] != ':' ){
				if( key_len == REQUEST_MAX_HEADER_KEY) {
					Log_Message("Header key max len");
					return 1;
				}
			}
			strncpy(header->key, line, key_len);
			if( line[key_len+1] == ' ' ) {
				strncpy(header->value, &line[key_len+2], REQUEST_MAX_HEADER_VALUE);
			}else{
				strncpy(header->value, &line[key_len+1], REQUEST_MAX_HEADER_VALUE);
			}

			request->headers[header_parsed++] = header;

			ptr = &ptr[path_cur+1];
			path_cur = 0;

			if(ptr[0] == '\r'){
				request->headers_count = header_parsed;
				header_parsed_done = 1;
			}
		}
	}

	if( ptr[1] == '\0' ){
		return 0; // no data
	}

	//parse data
	if( strncmp(ptr, "\r\n", 2)==0 ){
		ptr = &ptr[2];
		int data_parsed=0;
		int data_parsed_done=0;
		while (data_parsed < REQUEST_MAX_DATA_COUNT && data_parsed_done == 0){
			while( ptr[path_cur] != '\0' && ptr[path_cur] != '&'  && path_cur < REQUEST_MAX_DATA_RAW_KV  ){
				path_cur++;
			}

			Request_Data* data = malloc_c(sizeof(Request_Data));
			char *token;
			char line[REQUEST_MAX_DATA_RAW_KV]={0};
			char *search = "=";
			strncpy(line, ptr, path_cur);

			// key
			token = strtok(line, search);
			strncpy(data->key, token, REQUEST_DATA_KEY_LEN);

			// value
			token = strtok(NULL, search);
			strncpy(data->value, token, REQUEST_DATA_VALUE_LEN);

			request->data[data_parsed++] = data;

			if(ptr[path_cur] == '\0'){
				request->data_count = data_parsed;
				data_parsed_done = 1;
			}else{
				ptr = &ptr[path_cur+1];
				path_cur = 0;
			}
		}

		return 0;
	}

	return 0;
}
*/

void request_free(Request* request){
	for(int i=0;i<request->headers_count;i++){
		free_c(request->headers[i]);
	}

	for(int i=0;i<request->data_count;i++){
		free_c(request->data[i]);
	}


	//free_c(request);
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
