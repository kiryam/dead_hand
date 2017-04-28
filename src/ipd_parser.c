#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ipd_parser.h"
#include "common.h"

#define CURRENT_STATE() (enum state)parser->state
#define UPDATE_STATE(V) parser->state = (enum state) (V)

#define BUFFER_EMPTY()                                                \
do {                                                                  \
    memset(parser->buff, 0, sizeof(parser->buff) );                   \
	parser->buff_pos = 0;                                             \
} while(0)

#define BUFFER_PUSH(V) parser->buff[parser->buff_pos++] = V


void ipd_parser_init(ipd_parser *parser){
	 memset(parser, 0, sizeof(*parser));
	 parser->state = s_conn_id;
	 parser->buff_pos = 0;
	 BUFFER_EMPTY();
}

void ipd_parser_execute(ipd_parser *parser, char byte){
	parser->nread++;

	switch (CURRENT_STATE()) {
	case s_dead:
	        goto err;
	case s_conn_id:
		if (parser->buff_pos > 5) {
			parser->errno = CONNID_READ_ERROR;
			goto err;
			break;
		}
		if(byte == ','){
			parser->conn_id = atoi(parser->buff);
			UPDATE_STATE(s_message_size);
			BUFFER_EMPTY();
		} else {
			BUFFER_PUSH(byte);
		}

		break;
	case s_message_size:
		if (parser->buff_pos > 20) {
			parser->errno = MESSAGE_SIZE_READ_ERROR;
			goto err;
			break;
		}
		if(byte == ':'){
			parser->message_size = atoi(parser->buff);

			if( parser->message_size >= MAX_MESSAGE_SIZE ){
				parser->errno = MESSAGE_TOO_LONG;
				goto err;
			}

			parser->message = (char*) malloc_c(parser->message_size+1);
			if ( parser->message == NULL ){
				parser->errno = OUT_OF_MEMORY;
				goto err;
			}
			parser->message[parser->message_size] = '\0';
			UPDATE_STATE(s_message_read);
			BUFFER_EMPTY();
		} else {
			BUFFER_PUSH(byte);
		}

		break;

	case s_message_read: // FIXME
		parser->message[parser->message_bytes_readed++] = byte;

		if (parser->message_bytes_readed == parser->message_size) {
			parser->message[parser->message_bytes_readed] = '\0';
			UPDATE_STATE(s_message_done);
		}
		break;

	case s_message_done:
		return;
		break;
	}

	return;

	err:
	    if (parser->errno == OK) {
		   parser->errno = UNKNOWN;
	    }

	    if( parser->message != NULL ){
	    	free_c(parser->message);
	    }

	    return;
}

void ipd_parser_free(ipd_parser *parser){
	if (parser->message != NULL){
		free_c(parser->message);
	}
}



