#include "symbol.h"
#include <criterion/criterion.h>

Test(symbol_tests, it_interns_symbols) {
  symbol_intern_init();

  const char *sym1 = symbol_intern("foo");
  const char *sym2 = symbol_intern("bar");

  cr_assert_str_eq(sym1, "foo", "First symbol should be 'foo'");
  cr_assert_str_eq(sym2, "bar", "Second symbol should be 'bar'");

  symbol_intern_free_all();
}

Test(symbol_tests, it_returns_same_pointer_for_same_symbol) {
  symbol_intern_init();

  const char *sym1 = symbol_intern("baz");
  const char *sym2 = symbol_intern("baz");

  cr_assert_str_eq(sym1, "baz", "First symbol should be 'baz'");
  cr_assert_str_eq(sym2, "baz", "Second symbol should also be 'baz'");
  cr_assert_eq(sym1, sym2, "Both symbols should point to the same interned string");

  symbol_intern_free_all();
}

Test(symbol_tests, intern_returns_different_pointer_for_different_strings) {
  symbol_intern_init();
  const char *sym1 = symbol_intern("alpha");
  const char *sym2 = symbol_intern("beta");
  cr_assert_neq(sym1, sym2, "Different strings should have different interned pointers");
  symbol_intern_free_all();
}

Test(symbol_tests, intern_returns_null_for_null_input) {
  symbol_intern_init();
  const char *sym = symbol_intern(NULL);
  cr_assert_null(sym, "Interning NULL should return NULL");
  symbol_intern_free_all();
}

Test(symbol_tests, free_all_does_not_crash) {
  symbol_intern_init();
  symbol_intern("temp");
  symbol_intern_free_all();
  symbol_intern_init();
  const char *sym = symbol_intern("after_free");
  cr_assert_not_null(sym, "Interning after free_all should still work");
  symbol_intern_free_all();
}
