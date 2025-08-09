#include "lval.h"
#include <criterion/criterion.h>

Test(lval_tests, it_creates_number) {
  lval_t *lval = lval_num(42);
  cr_assert_not_null(lval, "lval should not be NULL");
  cr_assert_eq(lval->type, L_NUM, "lval type should be L_NUM");
  cr_assert_eq(lval->data.number, 42, "lval number should be 42");
  lval_free(lval);
}
