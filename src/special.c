#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "evaluator.h"
#include "lval.h"
#include "parser.h"
#include "special.h"

// Helpers
eval_result_t ast_atom_to_lval(const atom_t *a) {
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

static eval_result_t sf_quote(s_expression_t *list, env_t *env) {
  (void)env;
  if (list->data.list.count != 2) {
    return eval_errf("quote requires exactly one argument, got %zu", list->data.list.count - 1);
  }
  s_expression_t *arg = list->data.list.elements[1];
  if (arg->type == NODE_ATOM) {
    return ast_atom_to_lval(&arg->data.atom);
  }

  if (arg->type == NODE_LIST) {
    if (arg->data.list.count == 0 && arg->data.list.tail == NULL) {
      return eval_ok(lval_nil());
    }
    return eval_errf("quote: non-empty lists not supported yet");
  }
  return eval_errf("quote: expected atom or empty list");
}

// Lookups
typedef struct {
  const char *name;
  special_form_fn fn;
} special_entry_t;

static const special_entry_t k_specials[] = {
  { "quote", sf_quote },
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
