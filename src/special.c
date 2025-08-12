#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "evaluator.h"
#include "lval.h"
#include "parser.h"
#include "special.h"

// forward declarations
static eval_result_t ast_to_quoted_lval(const s_expression_t *e);
static eval_result_t ast_list_to_quoted_cons(const s_expression_t *list);

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

static eval_result_t ast_to_quoted_lval(const s_expression_t *e) {
  if (e->type == NODE_ATOM) {
    return ast_atom_to_quoted_lval(&e->data.atom);
  }
  if (e->type == NODE_LIST) {
    if (e->data.list.count == 0 && e->data.list.tail == NULL) {
      return eval_ok(lval_nil());
    }
    return ast_list_to_quoted_cons(e);
  }
  return eval_errf("quote: unknown node type %d", e->type);
}

static eval_result_t ast_list_to_quoted_cons(const s_expression_t *list) {
  lval_t *tail = NULL;
  if (list->data.list.tail) {
    eval_result_t tail_res = ast_to_quoted_lval(list->data.list.tail);
    if (tail_res.status != EVAL_OK) return tail_res;
    tail = tail_res.result;
  } else {
    tail = lval_nil();
  }

  for (ssize_t i = (ssize_t)list->data.list.count - 1; i >= 0; --i) {
    const s_expression_t *elem = list->data.list.elements[i];
    eval_result_t elem_res = ast_to_quoted_lval(elem);
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
  (void)env;
  if (list->data.list.count != 2) {
    return eval_errf("quote requires exactly one argument, got %zu", list->data.list.count - 1);
  }
  const s_expression_t *arg = list->data.list.elements[1];
  return ast_to_quoted_lval(arg);
}

static eval_result_t sf_unquote(s_expression_t *list, env_t *env) {
  (void)env;
  if (list->data.list.count != 2) {
    return eval_errf("unquote requires exactly one argument, got %zu", list->data.list.count - 1);
  }
  const eval_result_t arg_res =
}

// Lookups
typedef struct {
  const char *name;
  special_form_fn fn;
} special_entry_t;

static const special_entry_t k_specials[] = {
  { "quote", sf_quote }, { "unquote", sf_unquote },
  // {"if",     sf_if},
  // {"define", sf_define},
  // {"begin",  sf_begin},
  // {"lambda", sf_lambda},
};

special_form_fn lookup_special_form(const char *name) {
  for (size_t i = 0; i < sizeof k_specials / sizeof k_specials[0]; i++) {
    if (strcmp(name, k_specials[i].name) == 0) return k_specials[i].fn;
  }
  return NULL;
}
