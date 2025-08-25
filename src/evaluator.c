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

static s_expression_t *sexp_from_lval(const lval_t *v);
static void sexp_free_owned(s_expression_t *n);
static eval_result_t datum_from_sexp(const s_expression_t *e);

static s_expression_t *make_atom_symbol(const char *name) {
  s_expression_t *a = malloc(sizeof *a);
  if (!a) return NULL;
  a->type = NODE_ATOM;
  a->data.atom.type = ATOM_SYMBOL;
  a->data.atom.value.symbol = strdup(name);
  return a;
}
static s_expression_t *make_atom_string(const char *s) {
  s_expression_t *a = malloc(sizeof *a);
  if (!a) return NULL;
  a->type = NODE_ATOM;
  a->data.atom.type = ATOM_STRING;
  a->data.atom.value.string = strdup(s);
  return a;
}
static s_expression_t *make_atom_number(double x) {
  s_expression_t *a = malloc(sizeof *a);
  if (!a) return NULL;
  a->type = NODE_ATOM;
  a->data.atom.type = ATOM_NUMBER;
  a->data.atom.value.number = x;
  return a;
}
static s_expression_t *make_atom_boolean(bool b) {
  s_expression_t *a = malloc(sizeof *a);
  if (!a) return NULL;
  a->type = NODE_ATOM;
  a->data.atom.type = ATOM_BOOLEAN;
  a->data.atom.value.boolean = b;
  return a;
}
static s_expression_t *make_list_nodes(s_expression_t **elems, size_t n, s_expression_t *tail) {
  s_expression_t *l = malloc(sizeof *l);
  if (!l) return NULL;
  l->type = NODE_LIST;
  l->data.list.elements = elems;
  l->data.list.count = n;
  l->data.list.tail = tail;
  return l;
}

static s_expression_t *sexp_from_lval(const lval_t *v) {
  if (!v) return NULL;
  switch (v->type) {
  case L_NUM:
    return make_atom_number(v->as.number);
  case L_BOOL:
    return make_atom_boolean(v->as.boolean);
  case L_STRING:
    return make_atom_string(v->as.string.ptr);
  case L_SYMBOL:
    return make_atom_symbol(v->as.symbol.name);
  case L_NIL: {
    s_expression_t **none = malloc(0);
    return make_list_nodes(none, 0, NULL);
  }
  case L_CONS: {
    /* count proper prefix */
    size_t n = 0;
    const lval_t *cur = v;
    while (cur->type == L_CONS) {
      n++;
      cur = cur->as.cons.cdr;
    }
    s_expression_t *tail = NULL;
    if (cur->type != L_NIL) {
      tail = sexp_from_lval(cur);
      if (!tail) return NULL;
    }
    s_expression_t **elems = malloc(sizeof(*elems) * n);
    if (!elems) {
      if (tail) sexp_free_owned(tail);
      return NULL;
    }
    const lval_t *x = v;
    for (size_t i = 0; i < n; ++i) {
      elems[i] = sexp_from_lval(x->as.cons.car);
      if (!elems[i]) {
        for (size_t j = 0; j < i; ++j)
          sexp_free_owned(elems[j]);
        free(elems);
        if (tail) sexp_free_owned(tail);
        return NULL;
      }
      x = x->as.cons.cdr;
    }
    return make_list_nodes(elems, n, tail);
  }
  default:
    return NULL;
  }
}

static void sexp_free_owned(s_expression_t *n) {
  if (!n) return;
  switch (n->type) {
  case NODE_ATOM:
    if (n->data.atom.type == ATOM_SYMBOL && n->data.atom.value.symbol)
      free(n->data.atom.value.symbol);
    if (n->data.atom.type == ATOM_STRING && n->data.atom.value.string)
      free(n->data.atom.value.string);
    break;
  case NODE_LIST:
    for (size_t i = 0; i < n->data.list.count; ++i)
      sexp_free_owned(n->data.list.elements[i]);
    free(n->data.list.elements);
    sexp_free_owned(n->data.list.tail);
    break;
  }
  free(n);
}

static eval_result_t datum_from_sexp(const s_expression_t *e) {
  if (e->type == NODE_ATOM) {
    const atom_t *a = &e->data.atom;
    switch (a->type) {
    case ATOM_NUMBER:
      return eval_ok(lval_num(a->value.number));
    case ATOM_BOOLEAN:
      return eval_ok(lval_bool(a->value.boolean));
    case ATOM_STRING:
      return eval_ok(lval_string_copy(a->value.string, strlen(a->value.string)));
    case ATOM_SYMBOL:
      return eval_ok(lval_intern(a->value.symbol));
    default:
      return eval_errf("datum_from_sexp: bad atom");
    }
  }
  if (e->type == NODE_LIST) {
    lval_t *tail = e->data.list.tail ? NULL : lval_nil();
    if (e->data.list.tail) {
      eval_result_t t = datum_from_sexp(e->data.list.tail);
      if (t.status != EVAL_OK) return t;
      tail = t.result;
    }
    for (ssize_t i = (ssize_t)e->data.list.count - 1; i >= 0; --i) {
      eval_result_t it = datum_from_sexp(e->data.list.elements[i]);
      if (it.status != EVAL_OK) {
        if (tail) lval_free(tail);
        return it;
      }
      tail = lval_cons(it.result, tail);
    }
    return eval_ok(tail);
  }
  return eval_errf("datum_from_sexp: bad node");
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

static eval_result_t expand_macro_and_eval(lval_t *macro_fn, s_expression_t *call, env_t *env) {
  size_t argc = call->data.list.count - 1;
  lval_t **argv = NULL;
  if (argc) {
    argv = calloc(argc, sizeof(lval_t *));
    if (!argv) return eval_errf("Memory allocation failed for macro arguments array.");
    for (size_t i = 0; i < argc; i++) {
      eval_result_t res = datum_from_sexp(call->data.list.elements[i + 1]);
      if (res.status != EVAL_OK) {
        free(argv);
        return res;
      }
      argv[i] = res.result;
    }
  }

  eval_result_t res = evaluate_call(macro_fn, argc, argv, env);
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]) lval_free(argv[i]);
  }
  free(argv);
  if (res.status != EVAL_OK) return res;
  s_expression_t *expanded = sexp_from_lval(res.result);
  lval_free(res.result);
  if (!expanded) return eval_errf("macro: expansion is not compilable");
  eval_result_t out = evaluate_single(expanded, env);
  sexp_free_owned(expanded);
  return out;
}

eval_result_t evaluate_call(lval_t *fn, size_t argc, lval_t **argv, env_t *env) {
  if (!fn) return eval_errf("Unknown function");
  if (fn->type == L_SYMBOL) {
    const char *name = fn->as.symbol.name;
    lval_t *binding = env_get_ref(env, name);
    if (!binding) return eval_errf("Unknown function: %s", name);
    fn = binding;
  }

  if (fn->type != L_FUNCTION && fn->type != L_NATIVE) {
    return eval_errf("Expected a function, got: %s", lval_type_name(fn));
  }

  if (fn->type == L_NATIVE) {
    builtin_fn bf = (builtin_fn)fn->as.native.fn;
    if (!bf) return eval_errf("internal: null builtin");
    return bf(argc, argv, env);
  }

  if (argc != fn->as.function.param_count) {
    return eval_errf("Function expects %zu arguments, got %zu", fn->as.function.param_count, argc);
  }

  env_t *parent = fn->as.function.closure;
  if (!parent) {
    return eval_errf("internal: function has no closure");
  }
  env_t *call_env = env_new(parent);
  for (size_t i = 0; i < argc; i++) {
    if (!env_define(call_env, fn->as.function.params[i], lval_copy(argv[i]))) {
      env_release(call_env);
      return eval_errf("Failed to set parameter '%s' in function environment",
                       fn->as.function.params[i]);
    }
  }

  eval_result_t result = eval_ok(lval_nil());
  if (fn->as.function.body_count == 0) {
    env_release(call_env);
    return result;
  }
  for (size_t i = 0; i < fn->as.function.body_count; i++) {
    result = evaluate_single(fn->as.function.body[i], call_env);
    if (result.status != EVAL_OK) {
      env_release(call_env);
      return result;
    }
  }

  env_release(call_env);
  return result;
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
      if (sf) {
        return sf(expr, env);
      }
      lval_t *binding = env_get_ref(env, head_name);
      if (binding && binding->type == L_FUNCTION && binding->as.function.is_macro) {
        return expand_macro_and_eval(binding, expr, env);
      }
    }
    size_t argc = expr->data.list.count - 1;
    lval_t **argv = malloc(argc * sizeof(lval_t *));
    if (!argv) {
      return eval_errf("Memory allocation failed for arguments array.");
    }
    for (size_t i = 0; i < argc; i++) {
      eval_result_t res = evaluate_single(expr->data.list.elements[i + 1], env);
      if (res.status != EVAL_OK) {
        free(argv);
        return res;
      }
      argv[i] = res.result;
    }
    lval_t *callee = NULL;
    bool temp_sym = false;

    if (sexp_is_symbol(head, &head_name)) {
      callee = lval_intern(head_name);
      temp_sym = true;
    } else {
      eval_result_t res = evaluate_single(head, env);
      if (res.status != EVAL_OK) {
        free(argv);
        return res;
      }
      callee = res.result;
    }
    eval_result_t r = evaluate_call(callee, argc, argv, env);
    if (temp_sym) {
      lval_free(callee);
    }
    free(argv);
    return r;
  default:
    return eval_errf("Unknown expression type: %d", expr->type);
  }
}

eval_result_t evaluate_many(s_expression_t **exprs, size_t count, env_t *env) {
  if (!exprs || count == 0) return eval_ok(lval_nil());
  eval_result_t last = { 0 };

  for (size_t i = 0; i < count; i++) {
    eval_result_t r = evaluate_single(exprs[i], env);
    if (r.status != EVAL_OK) return r;
    last = r;
  }
  return last;
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
