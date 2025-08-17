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

typedef struct {
  const char *name;
  builtin_fn fn;
} builtin_entry_t;

// clang-format off
static const builtin_entry_t k_builtins[] = {
  { "+", builtin_add },
  { "-", builtin_sub },
  { "*", builtin_mul },
  { "/", builtin_div },
  { "mod", builtin_mod },
  { "abs", builtin_abs },
  { "min", builtin_min },
  { "max", builtin_max },
  // { "=", builtin_eq },
  // { "<", builtin_lt },
  // { ">", builtin_gt },
  // { "<=", builtin_le },
  // { ">=", builtin_ge },
};
// clang-format on
//
builtin_fn lookup_builtin(const char *name) {
  for (size_t i = 0; i < sizeof k_builtins / sizeof k_builtins[0]; i++) {
    if (strcmp(name, k_builtins[i].name) == 0) return k_builtins[i].fn;
  }
  return NULL;
}
