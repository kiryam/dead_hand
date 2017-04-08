#include "request.h"
#include "string.h"
#include "log.h"

int request_parse(Request* request, char* raw_header);

Request* request_init(char* raw_data){
	Request* request = (Request*)malloc_c(sizeof(Request));
	request->path[0] = '\0';
	request->proto[0] = '\0';
	request->headers_count = 0;
	request->data_count = 0;
	request->type =0;

	if( request_parse(request, raw_data) != 0){
		Log_Message("Parse header error");
		free_c(request);
		return NULL;
	}

	return request;
}


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

			// key
			token = strtok(line, search);
			strncpy(header->key, token, REQUEST_MAX_HEADER_KEY);

			// value
			token = strtok(NULL, search);
			strncpy(header->value, token, REQUEST_MAX_HEADER_VALUE);

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


void request_free(Request* request){
	for(int i=0;i<request->headers_count;i++){
		free_c(request->headers[i]);
	}

	for(int i=0;i<request->data_count;i++){
		free_c(request->data[i]);
	}


	free_c(request);
}
