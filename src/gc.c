#include "gc.h"
#include "env.h"
#include "lval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  lval_t *objects;
  struct env *global_env;
  size_t count;
  size_t trigger;
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
  G.count = 0;
  G.trigger = 100;
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
  G.count++;
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
  case L_STRING:
    break;
  default:
    break;
  }
}

static void gc_free_shallow(lval_t *v) {
  switch (v->type) {
  case L_STRING:
    free(v->as.string.ptr);
    break;
  case L_FUNCTION:
    if (v->as.function.params) {
      for (size_t i = 0; i < v->as.function.param_count; i++) {
        free(v->as.function.params[i]);
      }
      free(v->as.function.params);
    }
    if (v->as.function.closure) {
      env_release(v->as.function.closure);
    }
    break;
  default:
    break;
  }
  free(v);
}

static void gc_sweep(void) {
  lval_t **p = &G.objects;
  size_t survivors = 0;
  while (*p) {
    lval_t *v = *p;
    if (!v->mark) {
      *p = v->gc_next;
      gc_free_shallow(v);
      G.count--;
      continue;
    } else {
      v->mark = 0;
      survivors++;
      p = &v->gc_next;
    }
  }
  size_t base = 1024;
  size_t next = survivors < base ? base : survivors * 2;
  G.trigger = next;
}

void gc_collect(lval_t *extra_root) {
  if (G.global_env) env_gc_mark_all(G.global_env, gc_mark);
  if (extra_root) gc_mark(extra_root);
  for (gc_root_entry_t *entry = G_root_top; entry; entry = entry->next) {
    if (entry->slot && *entry->slot) {
      gc_mark(*entry->slot);
    }
  }
  gc_sweep();
}

void gc_maybe_collect() {
  if (G.count >= G.trigger) {
    gc_collect(NULL);
  }
}

void gc_root(lval_t **slot) {
  gc_root_entry_t *e = malloc(sizeof(gc_root_entry_t));
  if (!e) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  e->slot = slot;
  e->next = G_root_top;
  G_root_top = e;
  G_root_count++;
}

void gc_unroot(lval_t **slot) {
  gc_root_entry_t **p = &G_root_top;
  while (*p) {
    if ((*p)->slot == slot) {
      gc_root_entry_t *to_free = *p;
      *p = to_free->next;
      free(to_free);
      G_root_count--;
      return;
    }
    p = &(*p)->next;
  }
}

size_t gc_object_count(void) {
  return G.count;
}

void gc_set_trigger(size_t threshold) {
  G.trigger = threshold ? threshold : (size_t)-1;
}

void gc_reset(void) {
  while (G.objects) {
    lval_t *next = G.objects->gc_next;
    gc_free_shallow(G.objects);
    G.objects = next;
  }

  while (G_root_top) {
    gc_root_entry_t *next = G_root_top->next;
    free(G_root_top);
    G_root_top = next;
  }

  G.objects = NULL;
  G.count = 0;
  G.trigger = 100;
  G.global_env = NULL;
  G_root_count = 0;
}
