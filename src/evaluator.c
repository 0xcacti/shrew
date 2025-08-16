#include "evaluator.h"
#include "builtin.h"
#include "env.h"
#include "lval.h"
#include "parser.h"
#include "special.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool is_proper_call_list(const s_expression_t *list) {
  return list->data.list.tail == NULL;
}

static bool sexp_is_symbol(const s_expression_t *sexp, const char **out_name) {
  if (sexp->type != NODE_ATOM) return false;
  if (sexp->data.atom.type != ATOM_SYMBOL) return false;
  if (out_name) *out_name = sexp->data.atom.value.symbol;
  return true;
}

eval_result_t eval_ok(lval_t *result) {
  eval_result_t r = { 0 };
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
  eval_result_t r = { 0 };
  r.status = EVAL_ERR;
  r.error_message = msg;
  r.result = NULL;
  return r;
}

eval_result_t evaluate_single(s_expression_t *expr, env_t *env) {
  if (!expr) return eval_errf("Cannot evaluate a NULL expression.");

  switch (expr->type) {
  case NODE_ATOM: {
    atom_t *a = &expr->data.atom;
    switch (a->type) {
    case ATOM_NUMBER:
      return eval_ok(lval_num(a->value.number));
    case ATOM_BOOLEAN:
      return eval_ok(lval_bool(a->value.boolean));
    case ATOM_STRING: {
      const char *str = a->value.string;
      return eval_ok(lval_string_copy(a->value.string, strlen(str)));
    }
    case ATOM_SYMBOL: {
      const char *name = a->value.symbol;
      lval_t *found = env_get(env, name);
      if (found) return eval_ok(found);
      return eval_errf("Unbound symbol: %s", name);
    }
    default:
      return eval_errf("Unknown atom type: %d", a->type);
    }
  }
  case NODE_LIST:
    if (expr->data.list.count == 0 && expr->data.list.tail == NULL) {
      return eval_ok(lval_nil());
    }

    if (!is_proper_call_list(expr)) {
      return eval_errf("Dotted list cannot be used as a function call");
    }

    s_expression_t *head = expr->data.list.elements[0];
    const char *head_name = NULL;
    if (sexp_is_symbol(head, &head_name)) {
      special_form_fn sf = lookup_special_form(head_name);
      builtin_fn bf = lookup_builtin(head_name);
      if (sf) {
        return sf(expr, env);
      } else if (bf) {

        size_t argc = expr->data.list.count - 1;
        lval_t **argv = malloc(argc * sizeof(lval_t *));
        if (!argv) {
          return eval_errf("Memory allocation failed for function arguments.");
        }
        for (size_t i = 0; i < argc; i++) {
          eval_result_t res = evaluate_single(expr->data.list.elements[i + 1], env);
          if (res.status != EVAL_OK) {
            free(argv);
            return res;
          }
          argv[i] = res.result;
        }
        return bf(argc, argv, env);
      }
    }
    return eval_errf("Unknown function: %s", head_name ? head_name : "unknown");

  default:
    return eval_errf("Unknown expression type: %d", expr->type);
  }
}

void evaluator_result_free(eval_result_t *r) {
  if (!r) return;

  if (r->error_message != NULL) {
    free(r->error_message);
    r->error_message = NULL;
  }

  r->result = NULL;
  r->status = EVAL_OK;
}
