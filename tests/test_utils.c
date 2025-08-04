#include "utils.h"
#include <criterion/criterion.h>
#include <string.h>

// Helper to create an owned C‑string key for tests that survives until
// ht_destroy.
static char *dup(const char *s) {
  size_t n = strlen(s) + 1;
  char *p = malloc(n);
  cr_assert_not_null(p, "malloc failed");
  memcpy(p, s, n);
  return p;
}

Test(hashtable, init_and_destroy) {
  hashtable t;
  ht_error err = {0};
  cr_assert(ht_init(&t, 16, &err), "init failed: %s", err.error_message);
  cr_assert_eq(ht_count(&t), 0, "new table should be empty");
  ht_destroy(&t);
}

Test(hashtable, set_and_get_single) {
  hashtable t;
  ht_error err = {0};
  cr_assert(ht_init(&t, 4, &err));

  int value = 123;
  cr_assert(ht_set(&t, dup("foo"), &value, &err));

  void *out = NULL;
  cr_assert(ht_get(&t, "foo", &out));
  cr_assert_eq(*(int *)out, 123);

  ht_destroy(&t);
}

Test(hashtable, overwrite_updates_value_without_growth) {
  hashtable t;
  ht_error err = {0};
  cr_assert(ht_init(&t, 4, &err));

  int v1 = 1, v2 = 2;
  cr_assert(ht_set(&t, dup("dup"), &v1, &err));
  cr_assert(ht_set(&t, dup("dup"), &v2, &err));

  void *out = NULL;
  cr_assert(ht_get(&t, "dup", &out));
  cr_assert_eq(*(int *)out, 2);
  cr_assert_eq(ht_count(&t), 1, "overwrite shouldn’t change size");

  ht_destroy(&t);
}

Test(hashtable, erase_removes_binding) {
  hashtable t;
  ht_error err = {0};
  cr_assert(ht_init(&t, 4, &err));

  int v = 7;
  cr_assert(ht_set(&t, dup("tmp"), &v, &err));

  void *old = NULL;
  cr_assert(ht_erase(&t, "tmp", &old));
  cr_assert_eq(*(int *)old, 7);

  void *dummy = NULL;
  cr_assert(!ht_get(&t, "tmp", &dummy));
  cr_assert_eq(ht_count(&t), 0);

  ht_destroy(&t);
}

Test(hashtable, lots_of_inserts_force_resize) {
  const size_t N = 1000;
  hashtable t;
  ht_error err = {0};
  cr_assert(ht_init(&t, 4, &err));

  int *vals = malloc(sizeof(int) * N);
  for (size_t i = 0; i < N; ++i) {
    vals[i] = (int)i;
    char key[32];
    sprintf(key, "k%zu", i);
    cr_assert(ht_set(&t, dup(key), &vals[i], &err));
  }
  cr_assert_eq(ht_count(&t), N);

  for (size_t i = 0; i < N; ++i) {
    char key[32];
    sprintf(key, "k%zu", i);
    void *out = NULL;
    cr_assert(ht_get(&t, key, &out));
    cr_assert_eq(*(int *)out, (int)i);
  }

  free(vals);
  ht_destroy(&t);
}

Test(hashtable, iteration_visits_all_pairs) {
  hashtable t;
  ht_error err = {0};
  cr_assert(ht_init(&t, 8, &err));

  int a = 1, b = 2, c = 3;
  cr_assert(ht_set(&t, dup("a"), &a, &err));
  cr_assert(ht_set(&t, dup("b"), &b, &err));
  cr_assert(ht_set(&t, dup("c"), &c, &err));

  size_t visited = 0;
  ht_iter it;
  ht_iter_begin(&t, &it);
  const char *k;
  void *v;
  while (ht_iter_next(&it, &k, &v)) {
    visited += 1;
    if (strcmp(k, "a") == 0)
      cr_assert_eq(*(int *)v, 1);
    else if (strcmp(k, "b") == 0)
      cr_assert_eq(*(int *)v, 2);
    else if (strcmp(k, "c") == 0)
      cr_assert_eq(*(int *)v, 3);
    else
      cr_assert_fail("unexpected key %s", k);
  }
  cr_assert_eq(visited, 3);

  ht_destroy(&t);
}
