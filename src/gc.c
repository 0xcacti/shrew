#include "gc.h"
#include "env.h"
#include "lval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  lval_t *objects;
  struct env *global_env;
} gc_heap_t;

static gc_heap_t G = { 0 };

typedef struct gc_root_entry {
  lval_t **slot;
  struct gc_root_entry *next;
} gc_root_entry_t;

static gc_root_entry_t *G_root_top = NULL;
static size_t G_root_count = 0;

void gc_init(struct env *global_env) {
  G.global_env = global_env;
  G.objects = NULL;
  G_root_top = NULL;
  G_root_count = 0;
}

void gc_set_global_env(struct env *global_env) {
  G.global_env = global_env;
}

struct lval *gc_alloc_lval() {
  lval_t *v = malloc(sizeof(lval_t));
  if (!v) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  memset(v, 0, sizeof(*v));
  v->type = L_NIL;
  v->gc_next = G.objects;
  G.objects = v;
  return v;
}

static void gc_mark(lval_t *v);

static void gc_mark_cons(lval_t *v) {
  if (v->type != L_CONS) {
    fprintf(stderr, "gc_mark_cons: not a cons cell\n");
    return;
  }
  if (v->as.cons.car) gc_mark(v->as.cons.car);
  if (v->as.cons.cdr) gc_mark(v->as.cons.cdr);
}

static void gc_mark(lval_t *v) {
  if (!v || v->mark) return;
  v->mark = 1;

  switch (v->type) {
  case L_CONS:
    gc_mark_cons(v);
    break;
  case L_FUNCTION:
    if (v->as.function.closure) {
      env_gc_mark_all(v->as.function.closure, gc_mark);
    }
    break;
  default:
    break;
  }
}

static void gc_sweep(void) {
  lval_t **p = &G.objects;
  while (*p) {
    lval_t *v = *p;
    if (!v->mark) {
      *p = v->gc_next;
      lval_free(v);
      continue;
    } else {
      v->mark = 0;
      p = &v->gc_next;
    }
  }
}

void gc_collect(lval_t *extra_root) {
  if (G.global_env) env_gc_mark_all(G.global_env, gc_mark);
  if (extra_root) gc_mark(extra_root);
  gc_sweep();
}
