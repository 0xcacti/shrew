#ifndef ENV_H
#define ENV_H

#include "hashtable.h"
#include "lval.h"
#include <stdbool.h>

typedef struct env {
  struct env *parent; 
  hashtable *store;   
} env_t;

bool env_init(env_t *env, env_t *parent);
void env_destroy(env_t *env);
bool env_define(env_t *env, const char *key, lval_t *value);
bool env_set(env_t *env, const char *key, lval_t *value);
lval_t *env_get(env_t *env, const char *key);

#endif
