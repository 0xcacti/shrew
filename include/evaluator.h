#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "parser.h"
#include "utils.h"

struct hashtable;

typedef enum {
  EVAL_OK = 0, 
  EVAL_ERR = 1,
} eval_status_t;

typedef struct {
  eval_status_t status; 
  s_expression_t *result; 
  char *error_message; 
} eval_result_t;

typedef struct env {
  struct env *parent; 
  hashtable *store;   
} env_t;

s_expression_t *env_get(env_t *env, const char *key);

eval_result_t evaluate_single(s_expression_t *expr, env_t *env);
eval_result_t evaluatate_many(s_expression_t **exprs, size_t count, env_t *env);
void evaluator_result_free(eval_result_t *r);

#endif
