#ifndef ENV_H
#define ENV_H

#include "hashtable.h"
#include "lval.h"
#include <stdbool.h>

typedef struct env {
  struct env *parent; 
  hashtable *store;   
  size_t refcount;
  bool managed;
} env_t;

typedef void (*env_mark_fn)(lval_t *v);

env_t *env_new(env_t *parent);
void env_retain(env_t *env);
void env_release(env_t *env);
bool env_init(env_t *env, env_t *parent);
void env_destroy(env_t *env);
bool env_define(env_t *env, const char *key, lval_t *value);
bool env_set(env_t *env, const char *key, lval_t *value);
lval_t *env_get(env_t *env, const char *key);
lval_t *env_get_ref(env_t *env, const char *key);
void env_gc_mark_all(env_t *env, env_mark_fn mark_fn);

#endif
