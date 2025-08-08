#include "lval.h"
#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  v->as.string.ptr = malloc(len + 1);
  if (!v->as.string.ptr) {
    free(v);
    return NULL;
  }
  memcpy(v->as.string.ptr, s, len);
  v->as.string.ptr[len] = '\0';
  return v;
}

lval_t *lval_intern(const char *name) {
  lval_t *v = malloc(sizeof(lval_t));
  if (!v) return NULL;
  v->mark = 0;
  v->type = L_SYMBOL;

  const char *interned = symbol_intern(name);
  if (!interned) {
    free(v);
    return NULL;
  }

  v->as.symbol.name = interned;
  return v;
}

const char *lval_type_name(const lval_t *v) {
  switch (v->type) {
  case L_NUM:
    return "number";
  case L_STRING:
    return "string";
  case L_BOOL:
    return "boolean";
  case L_SYMBOL:
    return "symbol";
  default:
    return "unknown";
  }
}

void lval_print(const lval_t *v) {
  switch (v->type) {
  case L_NUM:
    printf("%g", v->as.number);
    break;
  case L_STRING:
    printf("\"%.*s\"", (int)v->as.string.len, v->as.string.ptr);
    break;
  case L_BOOL:
    printf(v->as.boolean ? "true" : "false");
    break;
  case L_SYMBOL:
    printf("%s", v->as.symbol.name);
    break;
  case L_NIL:
    printf("nil");
    break;
  default:
    printf("<unknown>");
    break;
  }
}
