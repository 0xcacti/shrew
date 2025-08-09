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

lval_t *lval_cons(lval_t *car, lval_t *cdr) {
  lval_t *v = malloc(sizeof(lval_t));
  if (!v) return NULL;
  v->mark = 0;
  v->type = L_CONS;
  v->as.cons.car = car;
  v->as.cons.cdr = cdr;
  return v;
}

lval_t *lval_nil(void) {
  lval_t *v = malloc(sizeof(lval_t));
  if (!v) return NULL;
  v->mark = 0;
  v->type = L_NIL;
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
  case L_NIL:
    return "nil";
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

lval_t *lval_copy(const lval_t *v) {
  if (!v) return NULL;

  lval_t *copy = malloc(sizeof *copy);
  if (!copy) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  copy->mark = 0;
  copy->type = v->type;

  switch (v->type) {
  case L_NUM:
    copy->as.number = v->as.number;
    break;

  case L_BOOL:
    copy->as.boolean = v->as.boolean;
    break;

  case L_STRING: {
    size_t len = v->as.string.len;
    copy->as.string.len = len;
    copy->as.string.ptr = malloc(len + 1);
    if (!copy->as.string.ptr) {
      perror("malloc");
      free(copy);
      exit(EXIT_FAILURE);
    }
    if (len) memcpy(copy->as.string.ptr, v->as.string.ptr, len);
    copy->as.string.ptr[len] = '\0';
  } break;

  case L_SYMBOL: {
    const char *name = v->as.symbol.name;
    lval_t *interned = lval_intern(name);
    free(copy);
    return interned;
  }

  case L_NIL:
    break;

  case L_CONS:
    copy->as.cons.car = lval_copy(v->as.cons.car);
    copy->as.cons.cdr = lval_copy(v->as.cons.cdr);
    break;

  default:
    fprintf(stderr, "lval_copy: unsupported type %d\n", (int)v->type);
    free(copy);
    exit(EXIT_FAILURE);
  }

  return copy;
}

void lval_free(lval_t *v) {
  if (!v) return;
  switch (v->type) {
  case L_STRING:
    free(v->as.string.ptr);
    break;
  case L_CONS:
    lval_free(v->as.cons.car);
    lval_free(v->as.cons.cdr);
    break;
  case L_SYMBOL:
  case L_NIL:
  case L_NUM:
  case L_BOOL:
  default:
    break;
  }
  free(v);
}
