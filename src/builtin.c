#include "builtin.h"
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

typedef struct {
  const char *name;
  builtin_fn fn;
} builtin_entry_t;

static const builtin_entry_t k_builtins[] = {
  { "+", builtin_add },
};

builtin_fn lookup_builtin(const char *name) {
  for (size_t i = 0; i < sizeof k_builtins / sizeof k_builtins[0]; i++) {
    if (strcmp(name, k_builtins[i].name) == 0) return k_builtins[i].fn;
  }
  return NULL;
}
