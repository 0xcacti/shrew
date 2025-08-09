#ifndef SPECIAL_H
#define SPECIAL_H

#include "parser.h" 
#include "lval.h"
#include "env.h"
#include "evaluator.h"

typedef eval_result_t (*special_form_fn)(s_expression_t *list, env_t *env);
special_form_fn lookup_special_form(const char *name);


#endif
