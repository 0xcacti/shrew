#ifndef BULTIN_H
#define BULTIN_H

#include "lval.h"
#include "env.h"
#include "evaluator.h"

typedef eval_result_t (*builtin_fn)(size_t argc, lval_t **argv, env_t *env);
builtin_fn lookup_builtin(const char *name);

#endif
