#include "gc.h"
#include "env.h"
#include "lval.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  lval_t *objects;
  struct env *global_env;
} gc_heap_t;

static gc_heap_t G = { 0 };

void gc_init(struct env *global_env) {
  G.global_env = global_env;
}
