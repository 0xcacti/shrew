#ifndef LVAL_H
#define LVAL_H
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  L_NIL,
  L_BOOL,
  L_NUM,
  L_STRING,
  L_SYMBOL,
  // add more here later
} ltype_t;

typedef struct lval {
  unsigned char mark;
  ltype_t type;
  union {
    double number;
    bool boolean;
    struct { char *ptr; size_t len; } string;
    struct { const char *name; } symbol;
  } as;
} lval_t;

lval_t *lval_num(double x);
lval_t *lval_bool(bool b);
lval_t *lval_string_copy(const char *s, size_t len);
lval_t *lval_intern(const char *name);
lval_t *lval_nil(void);
const char *lval_type_name(const lval_t *v);
void lval_print(const lval_t *v);
void lval_free(lval_t *v);

#endif 
