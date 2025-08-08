#include "lval.h"
#include <stdlib.h>

lval_t *lval_num(double x) {
  lval_t *v = malloc(sizeof(lval_t));
  if (!v) return NULL;
  v->mark = 0; // 0 for now when created
  v->type = L_NUM;
  v->as.number = x;
  return v;
}

lval_t *lval_bool(bool b) {
  lval_t *v = malloc(sizeof(lval_t));
  if (!v) return NULL;
  v->mark = 0; // 0 for now when created
  v->type = L_BOOL;
  v->as.boolean = b;
  return v;
}

lval_t *lval_string_copy(const char *s, size_t len) {
  lval_t *v = malloc(sizeof(lval_t));
  if (!v) return NULL;
  v->mark = 0; // 0 for now when created
  v->type = L_STRING;
  v->as.string.len = len;
  v->as.string.data = malloc(len + 1);
  if (!v->as.string.data) {
    free(v);
    return NULL;
  }
  memcpy(v->as.string.data, s, len);
  v->as.string.data[len] = '\0'; // null-terminate
  return v;
}

// lval_t *lval_string_copy(const char *s, size_t len) {}
// lval_t *lval_intern(const char *name) {}
// const char *lval_type_name(const lval_t *v) {}
// void lval_print(const lval_t *v) {}
