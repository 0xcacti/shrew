#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "evaluator.h"
#include "lval.h"
#include "parser.h"
#include "special.h"

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
  return eval_errf("unquote-splicing is only valid inside a quasiquote");
}

static eval_result_t sf_unquote_splicing(s_expression_t *list, env_t *env) {
  (void)env;
  (void)list;
  return eval_errf("unquote-splicing is only valid inside a quasiquote");
}

bool is_simple_form(const s_expression_t *e, const char *tag, const s_expression_t **out_arg) {
  if (!e || e->type != NODE_LIST) return false;
  if (e->data.list.tail != NULL) return false;
  if (e->data.list.count != 2) return false;
  const s_expression_t *first = e->data.list.elements[0];
  if (!sexp_is_symbol_name(first, tag)) return false;
  if (out_arg) *out_arg = e->data.list.elements[1];
  return true;
}

static lval_t *make_simple_list(const char *sym, lval_t *v) {
  lval_t *s = lval_intern(sym);
  lval_t *p = lval_cons(v, lval_nil());
  return lval_cons(s, p);
}

// forward declaration
static eval_result_t qq_expand_any(const s_expression_t *e, env_t *env, int depth);

static eval_result_t qq_expand_list(const s_expression_t *list, env_t *env, int depth) {
  lval_t *tail = NULL;
  if (list->data.list.tail) {
    const s_expression_t *arg = NULL;
    if (is_simple_form(list->data.list.tail, "unquote", &arg) && depth == 1) {
      eval_result_t r = evaluate_single((s_expression_t *)arg, env);
      if (r.status != EVAL_OK) return r;
      tail = r.result;
    } else if (is_simple_form(list->data.list.tail, "unquote-splicing", &arg) && depth == 1) {
      return eval_errf("unquote-splicing not allowed in dotted tail");
    } else if (is_simple_form(list->data.list.tail, "quasiquote", &arg)) {
      eval_result_t inner = qq_expand_any(arg, env, depth + 1);
      if (inner.status != EVAL_OK) return inner;
      tail = make_simple_list("quasiquote", inner.result);
    } else {
      eval_result_t r = qq_expand_any(list->data.list.tail, env, depth);
      if (r.status != EVAL_OK) return r;
      tail = r.result;
    }
  } else {
    tail = lval_nil();
  }

  for (ssize_t i = (ssize_t)list->data.list.count - 1; i >= 0; --i) {
    const s_expression_t *elem = list->data.list.elements[i];
    const s_expression_t *arg = NULL;
    if (is_simple_form(elem, "unquote", &arg)) {
      if (depth == 1) {
        eval_result_t r = evaluate_single((s_expression_t *)arg, env);
        if (r.status != EVAL_OK) {
          return r;
        }
        tail = lval_cons(r.result, tail);
      } else {
        eval_result_t inner = qq_expand_any(arg, env, depth - 1);
        if (inner.status != EVAL_OK) {
          return inner;
        }
        lval_t *form = make_simple_list("unquote", inner.result);
        tail = lval_cons(form, tail);
      }
      continue;
    }

    if (is_simple_form(elem, "unquote-splicing", &arg)) {
      if (depth == 1) {
        eval_result_t r = evaluate_single((s_expression_t *)arg, env);
        if (r.status != EVAL_OK) {
          return r;
        }
        if (r.result->type != L_CONS && r.result->type != L_NIL) {
          return eval_errf("unquote-splicing: expected list");
        }
        size_t n = 0;
        lval_t *cur = r.result;
        while (cur->type == L_CONS) {
          n++;
          cur = cur->as.cons.cdr;
        }
        if (cur->type != L_NIL) {
          return eval_errf("unquote-splicing: expected proper list");
        }
        if (n) {
          lval_t **elems = malloc(sizeof(lval_t *) * n);
          if (!elems) {
            return eval_errf("oom");
          }
          size_t k = 0;
          for (lval_t *x = r.result; x->type == L_CONS; x = x->as.cons.cdr)
            elems[k++] = x->as.cons.car;
          for (ssize_t j = (ssize_t)n - 1; j >= 0; --j)
            tail = lval_cons(lval_copy(elems[j]), tail);
          free(elems);
        }
      } else {
        eval_result_t inner = qq_expand_any(arg, env, depth - 1);
        if (inner.status != EVAL_OK) {
          return inner;
        }
        lval_t *form = make_simple_list("unquote-splicing", inner.result);
        tail = lval_cons(form, tail);
      }
      continue;
    }

    if (is_simple_form(elem, "quasiquote", &arg)) {
      eval_result_t inner = qq_expand_any(arg, env, depth + 1);
      if (inner.status != EVAL_OK) {
        return inner;
      }
      lval_t *form = make_simple_list("quasiquote", inner.result);
      tail = lval_cons(form, tail);
      continue;
    }

    eval_result_t v = qq_expand_any(elem, env, depth);
    if (v.status != EVAL_OK) {
      return v;
    }
    tail = lval_cons(v.result, tail);
  }
  return eval_ok(tail);
}

static eval_result_t qq_expand_any(const s_expression_t *e, env_t *env, int depth) {
  if (e->type == NODE_ATOM) {
    return ast_atom_to_quoted_lval(&e->data.atom);
  }
  if (e->type == NODE_LIST) {
    const s_expression_t *arg = NULL;
    if (is_simple_form(e, "quasiquote", &arg)) {
      eval_result_t inner = qq_expand_any(arg, env, depth + 1);
      if (inner.status != EVAL_OK) return inner;
      lval_t *form = make_simple_list("quasiquote", inner.result);
      return eval_ok(form);
    }
    if (e->data.list.count == 0 && e->data.list.tail == NULL) {
      return eval_ok(lval_nil());
    }
    return qq_expand_list(e, env, depth);
  }
  return eval_errf("quasiquote: unknown node type");
}

static eval_result_t sf_quasiquote(s_expression_t *list, env_t *env) {
  if (list->data.list.tail != NULL) return eval_errf("quasiquote: cannot have dotted arguments");
  if (list->data.list.count != 2) {
    return eval_errf("quasiquote requires exactly one argument, got %zu",
                     list->data.list.count - 1);
  }
  const s_expression_t *arg = list->data.list.elements[1];
  return qq_expand_any(arg, env, 1);
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
    return eval_errf("define: failed to define variable '%s'", name);
  }
  return eval_ok(lval_intern(name));
}

static eval_result_t sf_set(s_expression_t *list, env_t *env) {
  if (list->data.list.tail != NULL) return eval_errf("set: dotted form not allowed");
  if (list->data.list.count != 3) {
    return eval_errf("set requires exactly two arguments, got %zu", list->data.list.count - 1);
  }

  const s_expression_t *name_node = list->data.list.elements[1];
  if (name_node->type != NODE_ATOM || name_node->data.atom.type != ATOM_SYMBOL) {
    return eval_errf("set: first argument must be a symbol");
  }
  const char *name = name_node->data.atom.value.symbol;
  s_expression_t *value_expr = list->data.list.elements[2];
  eval_result_t value_res = evaluate_single(value_expr, env);
  if (value_res.status != EVAL_OK) {
    return value_res;
  }

  if (!env_set(env, name, value_res.result)) {
    return eval_errf("set: variable '%s' not defined", name);
  }
  return eval_ok(value_res.result);
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

  lval_t *fn = lval_function(params, param_count, body, body_count, env, false);
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

static eval_result_t sf_cond(s_expression_t *list, env_t *env) {
  if (list->data.list.tail != NULL) return eval_errf("cond: cannot have dotted arguments");
  if (list->data.list.count != 2) return eval_errf("cond: cond requires one argument");
  s_expression_t *cond_list = list->data.list.elements[1];
  if (cond_list->type != NODE_LIST) return eval_errf("cond: expects argument to be a list");
  if (cond_list->data.list.tail != NULL) return eval_errf("cond: cond list cannot be dotted");
  size_t llen = cond_list->data.list.count;
  if (llen % 2 != 0) return eval_errf("cond: improperly formatted cond list");
  for (size_t i = 0; i < llen; i += 2) {
    s_expression_t *to_check = cond_list->data.list.elements[i];
    eval_result_t cond_result = evaluate_single(to_check, env);
    if (cond_result.status != EVAL_OK) return cond_result;
    if (cond_result.result->type != L_BOOL)
      return eval_errf("cond: nonboolean condition encountered");
    bool obtains = cond_result.result->as.boolean;
    if (obtains) {
      return evaluate_single(cond_list->data.list.elements[i + 1], env);
    }
  }
  return eval_ok(lval_nil());
}

static eval_result_t sf_begin(s_expression_t *list, env_t *env) {
  if (list->data.list.tail != NULL) return eval_errf("begin: cannot have dotted arguments");
  size_t n = list->data.list.count;
  if (n <= 1) return eval_ok(lval_nil());
  return evaluate_many(&list->data.list.elements[1], n - 1, env);
}

static eval_result_t sf_defmacro(s_expression_t *list, env_t *env) {
  if (list->data.list.tail != NULL) return eval_errf("defmacro: cannot have dotted arguments");
  if (list->data.list.count < 3) {
    return eval_errf("defmacro: need a name and a lambda-ish body");
  }

  const s_expression_t *name_node = list->data.list.elements[1];
  if (name_node->type != NODE_ATOM || name_node->data.atom.type != ATOM_SYMBOL) {
    return eval_errf("defmacro: first argument must be a symbol");
  }

  const char *name = name_node->data.atom.value.symbol;

  s_expression_t *params_node = list->data.list.elements[2];
  if (params_node->type != NODE_LIST) {
    return eval_errf("defmacro: second argument must be a list of parameters");
  }
  if (params_node->data.list.tail != NULL) {
    return eval_errf("defmacro: parameter list cannot be dotted");
  }

  size_t param_count = params_node->data.list.count;
  char **params = NULL;
  if (param_count) {
    params = calloc(param_count, sizeof(char *));
    if (!params) return eval_errf("defmacro: memory allocation failed for parameters");
    for (size_t i = 0; i < param_count; ++i) {
      const s_expression_t *p = params_node->data.list.elements[i];
      if (p->type != NODE_ATOM || p->data.atom.type != ATOM_SYMBOL) {
        for (size_t j = 0; j < i; ++j) {
          free(params[j]);
        }
        free(params);
        return eval_errf("defmacro: parameter %zu is not a symbol", i + 1);
      }
      params[i] = strdup(p->data.atom.value.symbol);
      if (!params[i]) {
        for (size_t j = 0; j < i; ++j) {
          free(params[j]);
        }
        free(params);
        return eval_errf("defmacro: memory allocation failed for parameter '%s'",
                         p->data.atom.value.symbol);
      }
    }
  }

  size_t body_count = list->data.list.count - 3;
  s_expression_t **body = NULL;
  if (body_count) {
    body = calloc(body_count, sizeof(s_expression_t *));
    if (!body) {
      for (size_t i = 0; i < param_count; i++) {
        free(params[i]);
      }
      free(params);
      return eval_errf("defmacro: memory allocation failed for body");
    }
    for (size_t i = 0; i < body_count; ++i) {
      body[i] = list->data.list.elements[i + 3];
    }
  }

  lval_t *fn = lval_function(params, param_count, body, body_count, env, true);
  if (!fn) {
    for (size_t i = 0; i < param_count; i++) {
      free(params[i]);
    }
    free(params);
    free(body);
    return eval_errf("defmacro: failed to create macro");
  }

  if (!env_define(env, name, fn)) {
    return eval_errf("defmacro: failed to define macro '%s'", name);
  }

  return eval_ok(lval_intern(name));
}

typedef struct {
  const char *name;
  special_form_fn fn;
} special_entry_t;

// clang-format off
static const special_entry_t k_specials[] = {
  { "quote", sf_quote }, 
  { "unquote", sf_unquote },
  { "unquote-splicing", sf_unquote_splicing },
  { "quasiquote", sf_quasiquote },
  { "define", sf_define },
  { "set", sf_set },
  { "lambda", sf_lambda },
  { "if",     sf_if },
  { "cond", sf_cond },
  { "begin",  sf_begin },
  { "defmacro", sf_defmacro }

};
// clang-format on

special_form_fn lookup_special_form(const char *name) {
  for (size_t i = 0; i < sizeof k_specials / sizeof k_specials[0]; i++) {
    if (strcmp(name, k_specials[i].name) == 0) return k_specials[i].fn;
  }
  return NULL;
}
