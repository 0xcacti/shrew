#include "builtin.h"
#include <float.h>
#include <math.h>
#include <string.h>

static eval_result_t builtin_add(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  double s = 0.0;
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf("+: expected number at arg %zu", i + 1);
    }
    s += argv[i]->as.number;
  }
  return eval_ok(lval_num(s));
}

static eval_result_t builtin_sub(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  double s = 0.0;

  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf("+: expected number at arg %zu", i + 1);
    }
    if (i == 0) {
      s = argv[i]->as.number;
      continue;
    }
    s -= argv[i]->as.number;
  }
  return eval_ok(lval_num(s));
}

static eval_result_t builtin_mul(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  double s = 1.0;
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf("+: expected number at arg %zu", i + 1);
    }
    s *= argv[i]->as.number;
  }
  return eval_ok(lval_num(s));
}

static eval_result_t builtin_div(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  double s = 0.0;
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf("+: expected number at arg %zu", i + 1);
    }
    if (i == 0) {
      s = argv[i]->as.number;
      continue;
    }
    s /= argv[i]->as.number;
  }
  return eval_ok(lval_num(s));
}

static eval_result_t builtin_mod(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 2) {
    return eval_errf("mod: expected exactly 2 arguments, got %zu", argc);
  }
  if (argv[0]->type != L_NUM || argv[1]->type != L_NUM) {
    return eval_errf("mod: expected both arguments to be numbers");
  }
  if (argv[1]->as.number == 0.0) {
    return eval_errf("mod: division by zero");
  }
  double result = fmod(argv[0]->as.number, argv[1]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_abs(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("abs: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("abs: expected a number argument");
  }
  double result = fabs(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_min(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc == 0) {
    return eval_errf("min: expected at least 1 argument, got %zu", argc);
  }
  double min = DBL_MAX;
  for (size_t i = 0; i < argc; i++) {
    lval_t *arg = argv[i];
    if (arg->type != L_NUM) {
      return eval_errf("min: min on non-number type");
    }
    if (arg->as.number <= min) {
      min = arg->as.number;
    }
  }
  return eval_ok(lval_num(min));
}

static eval_result_t builtin_max(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc == 0) {
    return eval_errf("max: expected at least 1 argument, got %zu", argc);
  }
  double max = -DBL_MAX;
  for (size_t i = 0; i < argc; i++) {
    lval_t *arg = argv[i];
    if (arg->type != L_NUM) {
      return eval_errf("min: min on non-number type");
    }
    if (arg->as.number >= max) {
      max = arg->as.number;
    }
  }
  return eval_ok(lval_num(max));
}

static eval_result_t builtin_floor(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("floor: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("floor: expected a number argument");
  }
  double result = floor(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_ceil(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("ceil: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("ceil: expected a number argument");
  }
  double result = ceil(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_round(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("round: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("round: expected a number argument");
  }
  double result = round(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_trunc(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("trunc: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("trunc: expected a number argument");
  }
  double result = trunc(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_sqrt(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("sqrt: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("sqrt: expected a number argument");
  }
  if (argv[0]->as.number < 0.0) {
    return eval_errf("sqrt: cannot take square root of negative number");
  }
  double result = sqrt(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_exp(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("exp: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("exp: expected a number argument");
  }
  double result = exp(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_log(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("log: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_NUM) {
    return eval_errf("log: expected a number argument");
  }
  if (argv[0]->as.number <= 0.0) {
    return eval_errf("log: cannot take logarithm of non-positive number");
  }
  double result = log(argv[0]->as.number);
  return eval_ok(lval_num(result));
}

static eval_result_t builtin_eq(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc < 2) {
    return eval_errf("=: expected at least 2 arguments, got %zu", argc);
  }

  lval_t *first = argv[0];
  if (first->type != L_NUM) {
    return eval_errf("=: expected number at arg 1");
  }
  for (size_t i = 1; i < argc; i++) {
    lval_t *arg = argv[i];
    if (arg->type != L_NUM) {
      return eval_errf("=: expected number at arg %zu", i + 1);
    }
    if (arg->as.number != first->as.number) {
      return eval_ok(lval_bool(false));
    }
  }
  return eval_ok(lval_bool(true));
}

static eval_result_t builtin_lt(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc < 2) {
    return eval_errf("<: expected at least 2 arguments, got %zu", argc);
  }
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf("<: expected number arguments");
    }
  }
  for (size_t i = 0; i < argc - 1; i++) {
    if (!(argv[i]->as.number < argv[i + 1]->as.number)) {
      return eval_ok(lval_bool(false));
    }
  }
  return eval_ok(lval_bool(true));
}

static eval_result_t builtin_gt(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc < 2) {
    return eval_errf(">: expected at least 2 arguments, got %zu", argc);
  }
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf(">: expected number arguments");
    }
  }
  for (size_t i = 0; i < argc - 1; i++) {
    if (!(argv[i]->as.number > argv[i + 1]->as.number)) {
      return eval_ok(lval_bool(false));
    }
  }
  return eval_ok(lval_bool(true));
}

static eval_result_t builtin_le(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc < 2) {
    return eval_errf("<=: expected at least 2 arguments, got %zu", argc);
  }
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf("<=: expected number arguments");
    }
  }
  for (size_t i = 0; i < argc - 1; i++) {
    if (!(argv[i]->as.number <= argv[i + 1]->as.number)) {
      return eval_ok(lval_bool(false));
    }
  }
  return eval_ok(lval_bool(true));
}

static eval_result_t builtin_ge(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc < 2) {
    return eval_errf(">=: expected at least 2 arguments, got %zu", argc);
  }
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_NUM) {
      return eval_errf(">=: expected number arguments");
    }
  }
  for (size_t i = 0; i < argc - 1; i++) {
    if (!(argv[i]->as.number >= argv[i + 1]->as.number)) {
      return eval_ok(lval_bool(false));
    }
  }
  return eval_ok(lval_bool(true));
}

static eval_result_t builtin_identity_eq(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 2) {
    return eval_errf("eq?: expected exactly 2 arguments, got %zu", argc);
  }

  lval_t *a = argv[0];
  lval_t *b = argv[1];

  if (a->type != b->type) {
    return eval_ok(lval_bool(false));
  }

  bool identical = false;
  switch (a->type) {
  case L_NIL:
    identical = true;
    break;
  case L_NUM:
    identical = (a->as.number == b->as.number);
    break;

  case L_BOOL:
    identical = (a->as.boolean == b->as.boolean);
    break;

  case L_SYMBOL:
    identical = (strcmp(a->as.symbol.name, b->as.symbol.name) == 0);
    break;

  case L_STRING:
    identical = (a->as.string.ptr == b->as.string.ptr);
    break;

  case L_CONS:
    identical = (a == b);
    break;

  default:
    eval_errf("eq?: unsupported type for identity comparison: %s", lval_type_name(a));
  }

  return eval_ok(lval_bool(identical));
}

static bool deep_eq_helper(lval_t *a, lval_t *b) {
  if (a == b) {
    return true;
  }
  if (a->type != b->type) {
    return false;
  }
  switch (a->type) {
  case L_NIL:
    return true; // Both are nil, so they are equal.
  case L_NUM:
    return (a->as.number == b->as.number);
  case L_BOOL:
    return (a->as.boolean == b->as.boolean);
  case L_SYMBOL:
    return (strcmp(a->as.symbol.name, b->as.symbol.name) == 0);
  case L_STRING:
    if (a->as.string.len != b->as.string.len) {
      return false;
    }
    return (memcmp(a->as.string.ptr, b->as.string.ptr, a->as.string.len) == 0);
  case L_CONS:
    if (a->as.cons.car == NULL && b->as.cons.car == NULL) {
      return true;
    }
    if (a->as.cons.car == NULL || b->as.cons.car == NULL) {
      return false;
    }
    return deep_eq_helper(a->as.cons.car, b->as.cons.car) &&
           deep_eq_helper(a->as.cons.cdr, b->as.cons.cdr);
  default:
    return (a == b);
  }
}

static eval_result_t builtin_deep_eq(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 2) {
    return eval_errf("equal: expected exactly 2 arguments, got %zu", argc);
  }

  bool equal = deep_eq_helper(argv[0], argv[1]);
  return eval_ok(lval_bool(equal));
}

static eval_result_t builtin_not(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("not: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_BOOL) {
    return eval_errf("not: expected a boolean argument");
  }
  bool result = !argv[0]->as.boolean;
  return eval_ok(lval_bool(result));
}

static eval_result_t builtin_and(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc == 0) {
    return eval_errf("and: expected at least 1 argument, got %zu", argc);
  }

  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_BOOL) {
      return eval_errf("and: expected boolean arguments");
    }
    if (!argv[i]->as.boolean) {
      return eval_ok(lval_bool(false));
    }
  }
  return eval_ok(lval_bool(true));
}

static eval_result_t builtin_or(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc == 0) {
    return eval_errf("or: expected at least 1 argument, got %zu", argc);
  }
  for (size_t i = 0; i < argc; i++) {
    if (argv[i]->type != L_BOOL) {
      return eval_errf("or: expected boolean arguments");
    }
    if (argv[i]->as.boolean) {
      return eval_ok(lval_bool(true));
    }
  }
  return eval_ok(lval_bool(false));
}

static eval_result_t builtin_cons(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 2) {
    return eval_errf("cons: expected exactly 2 arguments, got %zu", argc);
  }
  lval_t *cons_cell = lval_cons(argv[0], argv[1]);
  return eval_ok(cons_cell);
}

static eval_result_t builtin_car(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("car: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_CONS) {
    return eval_errf("car: expected a cons cell");
  }
  if (argv[0]->as.cons.car == NULL) {
    return eval_errf("car: cons cell is empty");
  }
  lval_t *car_value = argv[0]->as.cons.car;
  return eval_ok(lval_copy(car_value));
}

static eval_result_t builtin_cdr(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("cdr: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_CONS) {
    return eval_errf("cdr: expected a cons cell");
  }
  if (argv[0]->as.cons.cdr == NULL) {
    return eval_errf("cdr: cons cell is empty");
  }
  lval_t *cdr_value = argv[0]->as.cons.cdr;
  return eval_ok(lval_copy(cdr_value));
}

static eval_result_t builtin_list(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc == 0) {
    return eval_ok(lval_nil());
  }

  lval_t *list = lval_nil();
  for (size_t i = argc; i > 0; i--) {
    lval_t *item = lval_copy(argv[i - 1]);
    list = lval_cons(item, list);
  }
  return eval_ok(list);
}

static eval_result_t builtin_length(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("length: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type == L_NIL) {
    return eval_ok(lval_num(0.0)); // Length of nil is 0
  }

  if (argv[0]->type != L_CONS) {
    return eval_errf("length: expected a cons cell");
  }
  size_t length = 0;
  lval_t *current = argv[0];
  while (current->type == L_CONS) {
    length++;
    current = current->as.cons.cdr;
    if (current == NULL) break; // Reached the end of the list
  }

  return eval_ok(lval_num((double)length));
}

static eval_result_t builtin_append(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc < 2) {
    return eval_errf("append: expected at least 2 arguments, got %zu", argc);
  }

  lval_t *result = lval_nil();
  for (size_t i = argc; i > 0; i--) {
    lval_t *item = lval_copy(argv[i - 1]);
    result = lval_cons(item, result);
  }

  lval_t *reversed = lval_nil();
  while (result->type == L_CONS) {
    reversed = lval_cons(result->as.cons.car, reversed);
    result = result->as.cons.cdr;
  }

  return eval_ok(reversed);
}

static eval_result_t builtin_reverse(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("reverse: expected exactly 1 argument, got %zu", argc);
  }

  if (argv[0]->type == L_NIL) {
    return eval_ok(lval_nil());
  }

  if (argv[0]->type != L_CONS) {
    return eval_errf("reverse: expected a cons cell");
  }

  lval_t *result = lval_nil();
  lval_t *current = argv[0];
  while (current->type == L_CONS) {
    result = lval_cons(current->as.cons.car, result);
    current = current->as.cons.cdr;
    if (current == NULL) break;
  }

  return eval_ok(result);
}

static eval_result_t builtin_is_null(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("null?: expected exactly 1 argument, got %zu", argc);
  }
  return eval_ok(lval_bool(argv[0]->type == L_NIL));
}

static eval_result_t builtin_is_pair(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("pair?: expected exactly 1 argument, got %zu", argc);
  }
  if (argv[0]->type != L_CONS) {
    return eval_ok(lval_bool(false));
  }
  return eval_ok(lval_bool(argv[0]->as.cons.car != NULL || argv[0]->as.cons.cdr != NULL));
}

static eval_result_t builtin_is_atom(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("atom?: expected exactly 1 argument, got %zu", argc);
  }
  return eval_ok(lval_bool(argv[0]->type != L_CONS && argv[0]->type != L_NIL));
}

static eval_result_t builtin_is_number(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("number?: expected exactly 1 argument, got %zu", argc);
  }
  return eval_ok(lval_bool(argv[0]->type == L_NUM));
}

static eval_result_t builtin_is_symbol(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("symbol?: expected exactly 1 argument, got %zu", argc);
  }
  return eval_ok(lval_bool(argv[0]->type == L_SYMBOL));
}

static eval_result_t builtin_is_string(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("string?: expected exactly 1 argument, got %zu", argc);
  }
  return eval_ok(lval_bool(argv[0]->type == L_STRING));
}

static eval_result_t builtin_is_function(size_t argc, lval_t **argv, env_t *env) {
  if (argc != 1) {
    return eval_errf("function?: expected exactly 1 argument, got %zu", argc);
  }

  bool is_function = false;

  if (argv[0]->type == L_SYMBOL) {
    builtin_fn fn = lookup_builtin(argv[0]->as.symbol.name);
    is_function = (fn != NULL);
    if (!is_function) {
      lval_t *binding = env_get_ref(env, argv[0]->as.symbol.name);
      is_function = (binding != NULL && binding->type == L_FUNCTION);
    }
  }

  return eval_ok(lval_bool(is_function));
}

static eval_result_t builtin_is_list(size_t argc, lval_t **argv, env_t *env) {
  (void)env;
  if (argc != 1) {
    return eval_errf("list?: expected exactly 1 argument, got %zu", argc);
  }
  return eval_ok(lval_bool(argv[0]->type == L_CONS || argv[0]->type == L_NIL));
}

typedef struct {
  const char *name;
  builtin_fn fn;
} builtin_entry_t;

// clang-format off
static const builtin_entry_t k_builtins[] = {
  // math
  { "+", builtin_add },
  { "-", builtin_sub },
  { "*", builtin_mul },
  { "/", builtin_div },
  { "mod", builtin_mod },
  { "abs", builtin_abs },
  { "min", builtin_min },
  { "max", builtin_max },
  { "floor", builtin_floor },
  { "ceil", builtin_ceil },
  { "round", builtin_round },
  { "trunc", builtin_trunc },
  { "sqrt", builtin_sqrt },
  { "exp", builtin_exp },
  { "log", builtin_log },
  // comparison 
  { "=", builtin_eq },
  { "<", builtin_lt },
  { ">", builtin_gt },
  { "<=", builtin_le },
  { ">=", builtin_ge },
  { "eq", builtin_identity_eq },
  { "equal", builtin_deep_eq },
  // boolean
  { "not", builtin_not },
  { "and", builtin_and },
  { "or", builtin_or },
  // lists
  { "cons", builtin_cons },
  { "car", builtin_car },
  { "cdr", builtin_cdr },
  { "list", builtin_list },
  { "length", builtin_length },
  { "append", builtin_append },
  { "reverse", builtin_reverse },
  // type checking
  { "null?", builtin_is_null },
  { "pair?", builtin_is_pair },
  { "atom?", builtin_is_atom },
  { "list?", builtin_is_list },
  { "number?", builtin_is_number },
  { "symbol?", builtin_is_symbol },
  { "string?", builtin_is_string },
  { "function?", builtin_is_function },
  // casting 
};
// clang-format on

builtin_fn lookup_builtin(const char *name) {
  for (size_t i = 0; i < sizeof k_builtins / sizeof k_builtins[0]; i++) {
    if (strcmp(name, k_builtins[i].name) == 0) return k_builtins[i].fn;
  }
  return NULL;
}
