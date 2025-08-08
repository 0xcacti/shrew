#include "env.h"
#include "hashtable.h"
#include "parser.h"
#include <stdlib.h>

bool env_init(env_t *env, env_t *parent) {
  if (!env)
    return false;
  env->parent = parent;
  env->store = (struct hashtable *)malloc(sizeof *env->store);
  if (!env->store)
    return false;

  hashtable ht;
  ht_error err = {0};
  ht_init(&ht, 16, &err);

  env->parent = parent;
  env->store = &ht;
}

// Test(hashtable, init_and_destroy) {
//   hashtable t;
//   ht_error err = {0};
//   cr_assert(ht_init(&t, 16, &err), "init failed: %s", err.error_message);
//   cr_assert_eq(ht_count(&t), 0, "new table should be empty");
//   ht_destroy(&t);
// }
