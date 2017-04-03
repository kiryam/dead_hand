#ifndef TCL_H
#define TCL_H


#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f2xx.h"
#include "stddef.h"

/* Token type and control flow constants */
enum { TCMD, TWORD, TPART, TERROR };
enum { FERROR, FNORMAL, FRETURN, FBREAK, FAGAIN };
typedef char tcl_value_t;

typedef int (*tcl_cmd_fn_t)(struct tcl *, tcl_value_t *, void *);


struct tcl {
  struct tcl_env *env;
  struct tcl_cmd *cmds;
  tcl_value_t *result;
};

int tcl_eval(struct tcl *tcl, const char *s, size_t len);
void tcl_destroy(struct tcl *tcl);
void tcl_init(struct tcl *tcl);
int tcl_length(tcl_value_t *v);
const char *tcl_string(tcl_value_t *v);

#ifdef __cplusplus
}
#endif

#endif
