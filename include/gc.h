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

void gc_root(struct lval **slot);
void gc_unroot(struct lval **slot);
size_t gc_object_count(void);
void gc_set_trigger(size_t threshold);

#endif
