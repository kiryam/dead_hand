#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ipd_parser.h"
#include "common.h"
#include "message_queue.h"

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
			Log_Message_FAST("conn_id ok");
			Log_Message_FAST(parser->buff);
			if( parser->buff[0] == 'D' ){
				//parser->conn_id = 100; // TODO?
				UPDATE_STATE(s_conn_id_data);
				BUFFER_EMPTY();
				break;
			}

			if ( !isdigit(parser->buff[0]) ){
				parser->errno = CONNID_PARSE_ERROR;
				goto err;
			}
			parser->conn_id = atoi(parser->buff);
			if (parser->conn_id > MAX_PENDING_CONNETION ){
				parser->errno = CONNID_OVERSIZED;
				goto err;
			}
			UPDATE_STATE(s_message_size);
			BUFFER_EMPTY();
		} else {
			BUFFER_PUSH(byte);
		}

		break;
	case s_conn_id_data:
		if (parser->buff_pos > 5) {
			parser->errno = DATA_CONNID_READ_ERROR;
			goto err;
			break;
		}
		if(byte == ','){
			Log_Message_FAST("conn_id data ok");
			Log_Message_FAST(parser->buff);

			parser->conn_id = atoi(parser->buff);
			if (parser->conn_id > MAX_PENDING_CONNETION ){
				parser->errno = CONNID_OVERSIZED;
				goto err;
			}

			parser->is_data = 1;

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
			Log_Message_FAST("message_size ok");
			Log_Message_FAST(parser->buff);

			if( parser->message_size >= MAX_MESSAGE_SIZE ){
				parser->errno = MESSAGE_TOO_LONG;
				goto err;
			}

			if( parser->message_size <= 0 ){
				parser->errno = MESSAGE_PARSE_ERRROR;
				goto err;
			}

			parser->message = (char*) malloc_c(parser->message_size+1);
			if ( parser->message == NULL ){
				parser->errno = OUT_OF_MEMORY;
				goto err;
			}
			//memset(parser->message, 0, parser->message_size+1);
			UPDATE_STATE(s_message_read);
			BUFFER_EMPTY();
		} else {
			BUFFER_PUSH(byte);
		}

		break;

	case s_message_read:
		parser->message[parser->message_bytes_readed++] = byte;

		if (parser->message_bytes_readed == parser->message_size) {
			Log_Message_FAST("readed data");
			parser->message[parser->message_size] = '\0';
		//	parser->message_size++; // NULL byte add
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



