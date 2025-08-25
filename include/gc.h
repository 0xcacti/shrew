#ifndef GC_H
#define GC_H

#include <stddef.h>
#include <stdbool.h>

struct lval;
struct env;

void gc_init(struct env *global_env);
void gc_set_global_env(struct env *global_env);
struct lval *gc_alloc_lval();
void gc_collect(struct lval *extra_root);
void env_gc_mark_all(struct env *e, void (*mark)(struct lval *));

#endif
