#include "env.h"
#include "builtin.h"
#include "hashtable.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void env_release(env_t *env) {
  if (!env || !env->managed) return;
  if (--env->refcount == 0) {
    env_t *p = env->parent;
    env_destroy(env);
    if (p && p->managed) env_release(p);
    free(env);
  }
}

void env_retain(env_t *env) {
  if (!env || !env->managed) return;
  env->refcount++;
}

env_t *env_new(env_t *parent) {
  env_t *env = malloc(sizeof *env);
  if (!env) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  env->parent = parent;
  env->refcount = 1;
  env->managed = true;
  env->store = malloc(sizeof *env->store);
  if (!env->store) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  ht_error err = (ht_error){ 0 };
  if (!ht_init(env->store, 16, &err)) {
    fprintf(stderr,
            "env_init: ht_init failed: %s\n",
            err.error_message ? err.error_message : "(unknown)");
    free(env->store);
    env->store = NULL;
    exit(EXIT_FAILURE);
  }
  if (parent && parent->managed) env_retain(parent);
  return env;
}

bool env_init(env_t *env, env_t *parent) {
  if (!env) return false;
  env->parent = parent;
  env->refcount = 0;
  env->managed = false;

  env->store = malloc(sizeof *env->store);
  if (!env->store) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  ht_error err = (ht_error){ 0 };
  if (!ht_init(env->store, 16, &err)) {
    fprintf(stderr,
            "env_init: ht_init failed: %s\n",
            err.error_message ? err.error_message : "(unknown)");
    free(env->store);
    env->store = NULL;
    exit(EXIT_FAILURE);
  }
  return true;
}

void env_destroy(env_t *env) {
  if (!env || !env->store) return;
  ht_destroy(env->store);
  free(env->store);
  env->store = NULL;
}

bool env_define(env_t *env, const char *key, lval_t *value) {
  if (!env || !env->store || !key || !value) return false;
  ht_error err = { 0 };
  if (!ht_set(env->store, key, value, &err)) {
    fprintf(stderr, "Error defining key '%s': %s\n", key, err.error_message);
    exit(EXIT_FAILURE);
  }
  return true;
}

bool env_set(env_t *env, const char *key, lval_t *value) {
  if (!env || !key) return false;
  for (env_t *e = env; e; e = e->parent) {
    void *tmp = NULL;
    if (ht_get(e->store, key, &tmp)) {
      ht_error err = { 0 };
      if (!ht_set(e->store, key, value, &err)) {
        fprintf(stderr, "Error setting key '%s': %s\n", key, err.error_message);
        exit(EXIT_FAILURE);
      }
      return true;
    }
  }
  return false;
}

lval_t *env_get(env_t *env, const char *key) {
  if (!env || !key) return NULL;
  for (env_t *e = env; e; e = e->parent) {
    void *value = NULL;
    if (ht_get(e->store, key, &value)) {
      return lval_copy((lval_t *)value);
    }
  }
  return NULL;
}

lval_t *env_get_ref(env_t *env, const char *key) {
  if (!env || !key) return NULL;
  for (env_t *e = env; e; e = e->parent) {
    void *value = NULL;
    if (ht_get(e->store, key, &value)) {
      return (lval_t *)value;
    }
  }
  return NULL;
}
