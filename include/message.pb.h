/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.7 at Thu Feb 23 01:06:44 2017. */

#ifndef PB_MESSAGE_PB_H_INCLUDED
#define PB_MESSAGE_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _Hello {
    int32_t Message;
/* @@protoc_insertion_point(struct:Hello) */
} Hello;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Hello_init_default                       {0}
#define Hello_init_zero                          {0}

/* Field tags (for use in manual encoding/decoding) */
#define Hello_Message_tag                        1

/* Struct field encoding specification for nanopb */
extern const pb_field_t Hello_fields[2];

/* Maximum encoded size of messages (where known) */
#define Hello_size                               11

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define MESSAGE_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
