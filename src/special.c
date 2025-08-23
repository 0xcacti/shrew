#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "evaluator.h"
#include "lval.h"
#include "parser.h"
#include "special.h"

// forward declarations
static eval_result_t ast_to_quoted_lval(const s_expression_t *e, env_t *env);
static eval_result_t ast_list_to_quoted_cons(const s_expression_t *list, env_t *env);

static eval_result_t ast_atom_to_quoted_lval(const atom_t *a) {
  switch (a->type) {
  case ATOM_NUMBER:
    return eval_ok(lval_num(a->value.number));
  case ATOM_BOOLEAN:
    return eval_ok(lval_bool(a->value.boolean));
  case ATOM_STRING: {
    const char *s = a->value.string;
    return eval_ok(lval_string_copy(s, strlen(s)));
  }
  case ATOM_SYMBOL:
    return eval_ok(lval_intern(a->value.symbol));
  default:
    return eval_errf("quote: unknown atom type %d", a->type);
  }
}

static eval_result_t ast_to_quoted_lval(const s_expression_t *e, env_t *env) {
  if (e->type == NODE_ATOM) {
    return ast_atom_to_quoted_lval(&e->data.atom);
  }
  if (e->type == NODE_LIST) {
    if (e->data.list.count == 0 && e->data.list.tail == NULL) {
      return eval_ok(lval_nil());
    }
    if (e->data.list.tail == NULL && e->data.list.count == 2) {
      const s_expression_t *head = e->data.list.elements[0];
      if (head->type == NODE_ATOM && head->data.atom.type == ATOM_SYMBOL &&
          strcmp(head->data.atom.value.symbol, "unquote") == 0) {
        return evaluate_single(e->data.list.elements[1], env);
      }
    }
    return ast_list_to_quoted_cons(e, env);
  }
  return eval_errf("quote: unknown node type %d", e->type);
}

static eval_result_t ast_list_to_quoted_cons(const s_expression_t *list, env_t *env) {
  lval_t *tail = NULL;
  if (list->data.list.tail) {
    eval_result_t tail_res = ast_to_quoted_lval(list->data.list.tail, env);
    if (tail_res.status != EVAL_OK) return tail_res;
    tail = tail_res.result;
  } else {
    tail = lval_nil();
  }

  for (ssize_t i = (ssize_t)list->data.list.count - 1; i >= 0; --i) {
    const s_expression_t *elem = list->data.list.elements[i];
    eval_result_t elem_res = ast_to_quoted_lval(elem, env);
    if (elem_res.status != EVAL_OK) {
      lval_free(tail);
      return elem_res;
    }
    lval_t *acc = lval_cons(elem_res.result, tail);
    tail = acc;
  }
  return eval_ok(tail);
}

static eval_result_t sf_quote(s_expression_t *list, env_t *env) {
  if (list->data.list.count != 2) {
    return eval_errf("quote requires exactly one argument, got %zu", list->data.list.count - 1);
  }
  const s_expression_t *arg = list->data.list.elements[1];
  return ast_to_quoted_lval(arg, env);
}

static eval_result_t sf_unquote(s_expression_t *list, env_t *env) {
  (void)env;
  (void)list;
  return eval_errf("unquote is only valid inside a quote form");
}

static eval_result_t sf_define(s_expression_t *list, env_t *env) {
  if (list->data.list.count != 3) {
    return eval_errf("define requires exactly two arguments, got %zu", list->data.list.count - 1);
  }

  const s_expression_t *name_node = list->data.list.elements[1];
  if (name_node->type != NODE_ATOM || name_node->data.atom.type != ATOM_SYMBOL) {
    return eval_errf("define: first argument must be a symbol");
  }
  const char *name = name_node->data.atom.value.symbol;
  s_expression_t *value_expr = list->data.list.elements[2];
  eval_result_t value_res = evaluate_single(value_expr, env);
  if (value_res.status != EVAL_OK) {
    return value_res;
  }

  if (!env_define(env, name, value_res.result)) {
    lval_free(value_res.result);
    return eval_errf("define: failed to define variable '%s'", name);
  }
  return eval_ok(lval_intern(name));
}

static eval_result_t sf_lambda(s_expression_t *list, env_t *env) {
  if (list->data.list.count < 3) {
    return eval_errf("lambda requires at least two arguments, got %zu", list->data.list.count - 1);
  }

  s_expression_t *params_node = list->data.list.elements[1];
  if (params_node->type != NODE_LIST) {
    return eval_errf("lambda: first argument must be a list of parameters");
  }

  if (params_node->data.list.tail != NULL) {
    return eval_errf("lambda: parameter list cannot be dotted");
  }

  size_t param_count = params_node->data.list.count;
  char **params = NULL;
  if (param_count > 0) {
    params = malloc(param_count * sizeof(char *));
    if (!params) return eval_errf("lambda: memory allocation failed for parameters");
    for (size_t i = 0; i < param_count; ++i) {
      const s_expression_t *param = params_node->data.list.elements[i];
      if (param->type != NODE_ATOM || param->data.atom.type != ATOM_SYMBOL) {
        for (size_t j = 0; j < i; ++j) {
          free(params[j]);
        }
        free(params);
        return eval_errf("lambda: parameter %zu is not a symbol", i + 1);
      }
      const char *param_name = param->data.atom.value.symbol;
      params[i] = strdup(param_name);
      if (!params[i]) {
        for (size_t j = 0; j < i; ++j) {
          free(params[j]);
        }
        free(params);
        return eval_errf("lambda: memory allocation failed for parameter '%s'", param_name);
      }
    }
  }

  size_t body_count = list->data.list.count - 2;
  s_expression_t **body = NULL;
  if (body_count > 0) {
    body = malloc(body_count * sizeof(s_expression_t *));
    if (!body) {
      for (size_t i = 0; i < param_count; i++) {
        free(params[i]);
      }
      free(params);
      return eval_errf("lambda: memory allocation failed for body");
    }
    for (size_t i = 0; i < body_count; ++i) {
      body[i] = list->data.list.elements[i + 2];
    }
  }

  lval_t *fn = lval_function(params, param_count, body, body_count, env);
  if (!fn) {
    for (size_t i = 0; i < param_count; i++) {
      free(params[i]);
    }
    free(params);
    free(body);
    return eval_errf("lambda: failed to create function");
  }

  return eval_ok(fn);
}

static eval_result_t sf_if(s_expression_t *list, env_t *env) {
  if (list->data.list.tail != NULL) return eval_errf("if: cannot have dotted arguments");
  if (list->data.list.count < 3 || list->data.list.count > 4)
    return eval_errf("if requires two or three arguments, got %zu", list->data.list.count - 1);
  s_expression_t *cond_expr = list->data.list.elements[1];
  s_expression_t *then_expr = list->data.list.elements[2];
  s_expression_t *else_expr = NULL;
  if (list->data.list.count == 4) else_expr = list->data.list.elements[3];

  eval_result_t cond_res = evaluate_single(cond_expr, env);
  if (cond_res.status != EVAL_OK) return cond_res;
  if (cond_res.result->type != L_BOOL)
    return eval_errf("if: condition did not evaluate to a boolean");

  bool cond = cond_res.result->as.boolean;
  if (cond) {
    return evaluate_single(then_expr, env);
  } else if (else_expr) {
    return evaluate_single(else_expr, env);
  } else {
    return eval_ok(lval_nil());
  }
}

// Lookups
typedef struct {
  const char *name;
  special_form_fn fn;
} special_entry_t;

// clang-format off
static const special_entry_t k_specials[] = {
  { "quote", sf_quote }, 
  { "unquote", sf_unquote },
  { "define", sf_define },
  { "lambda", sf_lambda },
  { "if",     sf_if},
  // {"define", sf_define},
  // {"begin",  sf_begin},
};
// clang-format on

special_form_fn lookup_special_form(const char *name) {
  for (size_t i = 0; i < sizeof k_specials / sizeof k_specials[0]; i++) {
    if (strcmp(name, k_specials[i].name) == 0) return k_specials[i].fn;
  }
  return NULL;
}
