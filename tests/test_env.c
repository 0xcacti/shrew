#include "env.h"
#include "lval.h"
#include <criterion/criterion.h>

Test(env_tests, initializes_env) {
  env_t env;
  cr_assert(env_init(&env, NULL), "Failed to initialize environment");
  cr_assert_not_null(env.store, "Environment store should not be NULL");
  cr_assert_null(env.parent, "Parent environment should be NULL");
  env_destroy(&env);
}

Test(env_tests, define_and_get_in_current_env) {
  env_t env;
  cr_assert(env_init(&env, NULL));
  lval_t *v = lval_num(42);

  cr_assert(env_define(&env, "x", v), "define should succeed");
  lval_t *got = env_get(&env, "x");
  cr_assert_not_null(got, "get should find x");
  cr_assert_eq(got->type, L_NUM);
  cr_assert(got->as.number == 42);

  env_destroy(&env);
  lval_free(v);
}

Test(env_tests, get_walks_parent_chain) {
  env_t parent, child;
  cr_assert(env_init(&parent, NULL));
  cr_assert(env_init(&child, &parent));

  lval_t *v = lval_num(7);
  cr_assert(env_define(&parent, "p", v));
  lval_t *got = env_get(&child, "p");
  cr_assert_not_null(got);
  cr_assert(got->as.number == 7);

  env_destroy(&child);
  env_destroy(&parent);
  lval_free(v);
}

Test(env_tests, define_shadows_parent) {
  env_t parent, child;
  cr_assert(env_init(&parent, NULL));
  cr_assert(env_init(&child, &parent));

  lval_t *vp = lval_num(1);
  lval_t *vc = lval_num(2);
  cr_assert(env_define(&parent, "x", vp));
  cr_assert(env_define(&child, "x", vc));

  lval_t *got_child = env_get_ref(&child, "x");
  cr_assert(got_child == vc, "child should see its own binding");

  lval_t *got_parent = env_get_ref(&parent, "x");
  cr_assert(got_parent == vp, "parent binding should be unchanged");

  env_destroy(&child);
  env_destroy(&parent);
  lval_free(vp);
  lval_free(vc);
}

Test(env_tests, set_updates_nearest_existing_binding) {
  env_t parent, child;
  cr_assert(env_init(&parent, NULL));
  cr_assert(env_init(&child, &parent));

  lval_t *oldp = lval_num(10);
  lval_t *newp = lval_num(11);
  cr_assert(env_define(&parent, "y", oldp));

  cr_assert(env_set(&child, "y", newp), "set should update parent binding");

  lval_t *got_parent = env_get_ref(&parent, "y");
  cr_assert(got_parent == newp);

  lval_t *got_child = env_get_ref(&child, "y");
  cr_assert(got_child == newp);

  env_destroy(&child);
  env_destroy(&parent);
  lval_free(oldp);
  lval_free(newp);
}

Test(env_tests, set_fails_when_unbound_anywhere) {
  env_t env;
  cr_assert(env_init(&env, NULL));
  lval_t *v = lval_num(99);

  cr_assert(!env_set(&env, "does_not_exist", v), "set should fail if name is unbound");

  env_destroy(&env);
  lval_free(v);
}
