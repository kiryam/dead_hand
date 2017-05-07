#ifndef IPD_PARSER_H
#define IPD_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "message_queue.h"

#define IPD_BUFF 64
#define MAX_MESSAGE_SIZE 1024*4

typedef struct ipd_parser ipd_parser;


enum errno {
	  OK
	, UNKNOWN
	, CONNID_READ_ERROR
	, MESSAGE_SIZE_READ_ERROR
	, MESSAGE_TOO_LONG
	, MESSAGE_PARSE_ERRROR
	, OUT_OF_MEMORY
	, CONNID_OVERSIZED
	, CONNID_PARSE_ERROR
	, DATA_CONNID_READ_ERROR
};


enum state
  { s_dead = 1 /* important that this is > 0 */
	, s_conn_id
	, s_conn_id_data
	, s_message_size
	, s_message_read
    , s_message_done
  };


struct ipd_parser {
  unsigned int state;        /* enum state from http_parser.c */
  int nread;          /* # bytes read in various scenarios */
  unsigned int errno;
  uint8_t is_data;

  //unsigned int conn_id;

  char buff[IPD_BUFF];
  unsigned int buff_pos;

 // unsigned int message_size;
  unsigned int message_bytes_readed;
  //char message[MAX_MESSAGE_SIZE];

  message_data* packet;
};


#define HTTP_PARSER_ERRNO(p)            ((enum errno) (p)->errno)

void ipd_parser_init(ipd_parser *parser, message_data* packet);
void ipd_parser_free(ipd_parser *parser);
#ifdef __cplusplus
}
#endif

#endif

