#include "evaluator.h"
#include "hashtable.h"
#include "parser.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

eval_result_t eval_ok(s_expression_t *result) {
  eval_result_t r = {0};
  r.status = EVAL_OK;
  r.result = result;
  return r;
}

eval_result_t eval_errf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char tmp[256];
  int needed = vsnprintf(tmp, sizeof tmp, fmt, ap);
  va_end(ap);
  if (needed < 0) {
    perror("vsnprintf");
    exit(EXIT_FAILURE);
  }

  char *msg = malloc((size_t)needed + 1);
  if (!msg) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  va_start(ap, fmt);
  vsnprintf(msg, (size_t)needed + 1, fmt, ap);
  va_end(ap);
  eval_result_t r = {0};
  r.status = EVAL_ERR;
  r.error_message = msg;
  r.result = NULL;
  return r;
}

s_expression_t *env_get(env_t *env, const char *key) {
  if (!env || !key)
    return NULL;

  for (env_t *e = env; e; e = e->parent) {
    void *tmp = NULL;
    if (ht_get(e->store, key, &tmp)) {
      return (s_expression_t *)tmp;
    }
  }
  return NULL;
}

eval_result_t evaluate_single(s_expression_t *expr, env_t *env) {
  switch (expr->type) {
  case NODE_ATOM:
    switch (expr->data.atom.type) {
    case ATOM_NUMBER:
    case ATOM_BOOLEAN:
    case ATOM_STRING:
      return eval_ok(expr);
    case ATOM_SYMBOL: {
      s_expression_t *found = env_get(env, expr->data.atom.value.symbol);
      if (found) {
        return eval_ok(found);
      } else {
        return eval_errf("Unbound symbol: %s", expr->data.atom.value.symbol);
      }
    }
    }
  case NODE_LIST:
    fprintf(stderr, "List evaluation not implemented yet.\n");
    exit(EXIT_FAILURE); // TODO: Implement list evaluation
  }
}

void evaluator_result_free(eval_result_t *r) {
  if (!r)
    return;

  if (r->error_message != NULL) {
    free(r->error_message);
    r->error_message = NULL;
  }

  r->result = NULL;
  r->status = EVAL_OK;

  free(r);
}
