#include "lval.h"
#include "env.h"
#include "gc.h"
#include "symbol.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lval_t *lval_num(double x) {
  lval_t *v = gc_alloc_lval();
  if (!v) return NULL;
  v->type = L_NUM;
  v->as.number = x;
  return v;
}

lval_t *lval_bool(bool b) {
  lval_t *v = gc_alloc_lval();
  if (!v) return NULL;
  v->type = L_BOOL;
  v->as.boolean = b;
  return v;
}

lval_t *lval_string_copy(const char *s, size_t len) {

  char *buf = malloc(len + 1);
  if (!buf) return NULL;
  lval_t *v = gc_alloc_lval();
  if (!v) {
    free(buf);
    return NULL;
  }
  v->type = L_STRING;
  v->as.string.len = len;
  v->as.string.ptr = buf;
  memcpy(v->as.string.ptr, s, len);
  v->as.string.ptr[len] = '\0';
  return v;
}

lval_t *lval_intern(const char *name) {
  const char *interned = symbol_intern(name);
  if (!interned) return NULL;
  lval_t *v = gc_alloc_lval();
  if (!v) return NULL;
  v->type = L_SYMBOL;
  v->as.symbol.name = interned;
  return v;
}

lval_t *lval_cons(lval_t *car, lval_t *cdr) {
  lval_t *v = gc_alloc_lval();
  if (!v) return NULL;
  v->type = L_CONS;
  v->as.cons.car = car;
  v->as.cons.cdr = cdr;
  return v;
}

lval_t *lval_nil(void) {
  lval_t *v = gc_alloc_lval();
  if (!v) return NULL;
  v->type = L_NIL;
  return v;
}

lval_t *lval_function(char **params,
                      size_t param_count,
                      s_expression_t **body,
                      size_t body_count,
                      struct env *closure,
                      bool is_macro) {
  lval_t *v = gc_alloc_lval();
  if (!v) return NULL;
  v->type = L_FUNCTION;

  v->as.function.params = params;
  v->as.function.param_count = param_count;
  v->as.function.body = body;
  v->as.function.body_count = body_count;
  v->as.function.closure = closure;
  v->as.function.is_macro = is_macro;
  if (closure) env_retain(closure);

  return v;
}

lval_t *lval_native(void *fn, const char *name) {
  lval_t *v = gc_alloc_lval();
  if (!v) return NULL;
  v->type = L_NATIVE;
  v->as.native.fn = fn;
  v->as.native.name = name;
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
  case L_CONS:
    return "cons";
  case L_FUNCTION:
    return "function";
  case L_NATIVE:
    return "builtin";
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
  case L_CONS:
    printf("(");
    if (v->as.cons.car) {
      lval_print(v->as.cons.car);
    } else {
      printf("nil");
    }
    if (v->as.cons.cdr) {
      printf(" . ");
      lval_print(v->as.cons.cdr);
    }
    printf(")");
    break;
  case L_FUNCTION:
    printf("<function>");
    break;
  case L_NATIVE:
    if (v->as.native.name) {
      printf("<builtin:%s>", v->as.native.name);
    } else {
      printf("<builtin>");
    }
    break;
  default:
    printf("<unknown>");
    break;
  }
}

lval_t *lval_copy(const lval_t *v) {
  if (!v) return NULL;
  switch (v->type) {
  case L_NUM: {
    lval_t *o = gc_alloc_lval();
    o->type = L_NUM;
    o->as.number = v->as.number;
    return o;
  }
  case L_BOOL: {
    lval_t *o = gc_alloc_lval();
    o->type = L_BOOL;
    o->as.boolean = v->as.boolean;
    return o;
  }
  case L_STRING: {
    size_t len = v->as.string.len;
    char *buf = malloc(len + 1);
    if (!buf) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    if (len) memcpy(buf, v->as.string.ptr, len);
    buf[len] = '\0';
    lval_t *o = gc_alloc_lval();
    o->type = L_STRING;
    o->as.string.len = len;
    o->as.string.ptr = buf;
    return o;
  }
  case L_SYMBOL: {
    lval_t *o = gc_alloc_lval();
    o->type = L_SYMBOL;
    o->as.symbol.name = v->as.symbol.name;
    return o;
  }
  case L_NIL: {
    lval_t *o = gc_alloc_lval();
    o->type = L_NIL;
    return o;
  }
  case L_CONS: {
    lval_t *o = gc_alloc_lval();
    o->type = L_CONS;
    o->as.cons.car = lval_copy(v->as.cons.car);
    o->as.cons.cdr = lval_copy(v->as.cons.cdr);
    return o;
  }
  case L_FUNCTION: {
    lval_t *o = gc_alloc_lval();
    o->type = L_FUNCTION;
    o->as.function.param_count = v->as.function.param_count;
    if (v->as.function.param_count) {
      o->as.function.params = malloc(v->as.function.param_count * sizeof(char *));
      if (!o->as.function.params) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      for (size_t i = 0; i < v->as.function.param_count; i++) {
        o->as.function.params[i] = strdup(v->as.function.params[i]);
        if (!o->as.function.params[i]) {
          perror("strdup");
          exit(EXIT_FAILURE);
        }
      }
    } else {
      o->as.function.params = NULL;
    }
    o->as.function.body_count = v->as.function.body_count;
    if (v->as.function.body_count) {
      o->as.function.body = malloc(v->as.function.body_count * sizeof(s_expression_t *));
      if (!o->as.function.body) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      memcpy(o->as.function.body,
             v->as.function.body,
             v->as.function.body_count * sizeof(s_expression_t *));
    } else {
      o->as.function.body = NULL;
    }
    o->as.function.closure = v->as.function.closure;
    if (o->as.function.closure) env_retain(o->as.function.closure);
    o->as.function.is_macro = v->as.function.is_macro;
    return o;
  }
  case L_NATIVE: {
    lval_t *o = gc_alloc_lval();
    o->type = L_NATIVE;
    o->as.native.fn = v->as.native.fn;
    o->as.native.name = v->as.native.name;
    return o;
  }
  default:
    fprintf(stderr, "lval_copy: unsupported type %d\n", (int)v->type);
    exit(EXIT_FAILURE);
  }
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
  case L_FUNCTION:
    for (size_t i = 0; i < v->as.function.param_count; i++) {
      free(v->as.function.params[i]);
    }
    free(v->as.function.params);
    free(v->as.function.body);
    if (v->as.function.closure) env_release(v->as.function.closure);
    break;
  case L_SYMBOL:
  case L_NIL:
  case L_NUM:
  case L_BOOL:
  case L_NATIVE:
  default:
    break;
  }
  free(v);
}
