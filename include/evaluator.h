#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "parser.h"
#include "env.h"

typedef enum {
  EVAL_OK = 0, 
  EVAL_ERR = 1,
} eval_status_t;

typedef struct {
  eval_status_t status; 
  lval_t *result; 
  char *error_message; 
} eval_result_t;



eval_result_t eval_ok(lval_t *result);
eval_result_t eval_errf(const char *fmt, ...);
eval_result_t evaluate_single(s_expression_t *expr, env_t *env);
eval_result_t evaluatate_many(s_expression_t **exprs, size_t count, env_t *env);
void evaluator_result_free(eval_result_t *r);

#endif
