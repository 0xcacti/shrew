#include "lval.h"
#include "symbol.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>

Test(lval_tests, it_creates_number) {
  symbol_intern_init();
  lval_t *lval = lval_num(42);
  cr_assert_not_null(lval, "lval should not be NULL");
  cr_assert_eq(lval->type, L_NUM, "lval type should be L_NUM");
  cr_assert_eq(lval->as.number, 42, "lval number should be 42");
  lval_free(lval);
  symbol_intern_free_all();
}

Test(lval_tests, it_creates_symbol) {
  symbol_intern_init();
  lval_t *lval = lval_intern("test_symbol");
  cr_assert_not_null(lval, "lval should not be NULL");
  cr_assert_eq(lval->type, L_SYMBOL, "lval type should be L_SYMBOL");
  cr_assert_str_eq(lval->as.symbol.name, "test_symbol", "lval symbol name should be 'test_symbol'");
  lval_free(lval);
  symbol_intern_free_all();
}

Test(lval_tests, it_creates_string) {
  symbol_intern_init();
  lval_t *lval = lval_string_copy("test_string", 11);
  cr_assert_not_null(lval, "lval should not be NULL");
  cr_assert_eq(lval->type, L_STRING, "lval type should be L_STRING");
  cr_assert_str_eq(lval->as.string.ptr, "test_string", "lval string should be 'test_string'");
  lval_free(lval);
  symbol_intern_free_all();
}

Test(lval_tests, it_creates_nil) {
  symbol_intern_init();
  lval_t *lval = lval_nil();
  cr_assert_not_null(lval, "lval should not be NULL");
  cr_assert_eq(lval->type, L_NIL, "lval type should be L_NIL");
  lval_free(lval);
  symbol_intern_free_all();
}

Test(lval_tests, it_creates_bool) {
  symbol_intern_init();
  lval_t *lval_true = lval_bool(true);
  cr_assert_not_null(lval_true, "lval should not be NULL");
  cr_assert_eq(lval_true->type, L_BOOL, "lval type should be L_BOOL");
  cr_assert_eq(lval_true->as.boolean, true, "lval boolean should be true");

  lval_t *lval_false = lval_bool(false);
  cr_assert_not_null(lval_false, "lval should not be NULL");
  cr_assert_eq(lval_false->type, L_BOOL, "lval type should be L_BOOL");
  cr_assert_eq(lval_false->as.boolean, false, "lval boolean should be false");

  lval_free(lval_true);
  lval_free(lval_false);
  symbol_intern_free_all();
}

Test(lval_tests, it_creates_cons) {
  symbol_intern_init();
  lval_t *car = lval_num(42);
  lval_t *cdr = lval_string_copy("test_string", 11);
  lval_t *cons = lval_cons(car, cdr);

  cr_assert_not_null(cons, "lval cons should not be NULL");
  cr_assert_eq(cons->type, L_CONS, "lval type should be L_CONS");
  cr_assert_eq(cons->as.cons.car->type, L_NUM, "car type should be L_NUM");
  cr_assert_eq(cons->as.cons.cdr->type, L_STRING, "cdr type should be L_STRING");
  cr_assert_eq(cons->as.cons.car->as.number, 42, "car number should be 42");
  cr_assert_str_eq(
      cons->as.cons.cdr->as.string.ptr, "test_string", "cdr string should be 'test_string'");

  lval_free(cons);
  symbol_intern_free_all();
}

Test(lval_tests, it_gets_type_name) {
  symbol_intern_init();
  lval_t *num = lval_num(42);
  lval_t *sym = lval_intern("test_symbol");
  lval_t *str = lval_string_copy("test_string", 11);
  lval_t *nil = lval_nil();
  lval_t *t = lval_bool(true);
  lval_t *f = lval_bool(false);

  cr_assert_str_eq(lval_type_name(num), "number", "Type name should be 'number'");
  cr_assert_str_eq(lval_type_name(sym), "symbol", "Type name should be 'symbol'");
  cr_assert_str_eq(lval_type_name(str), "string", "Type name should be 'string'");
  cr_assert_str_eq(lval_type_name(nil), "nil", "Type name should be 'nil'");
  cr_assert_str_eq(lval_type_name(t), "boolean", "Type name should be 'boolean'");
  cr_assert_str_eq(lval_type_name(f), "boolean", "Type name should be 'boolean'");

  lval_free(num);
  lval_free(sym);
  lval_free(str);
  lval_free(nil);
  lval_free(t);
  lval_free(f);

  symbol_intern_free_all();
}

static void redirect_stdout(void) {
  cr_redirect_stdout();
}

Test(lval_tests, it_creates_function) {
  symbol_intern_init();

  char **params = malloc(2 * sizeof(char *));
  params[0] = strdup("x");
  params[1] = strdup("y");

  lval_t *body = lval_intern("body_expr");

  lval_t *func = lval_function(params, 2, body, NULL);

  cr_assert_not_null(func, "function lval should not be NULL");
  cr_assert_eq(func->type, L_FUNCTION, "lval type should be L_FUNCTION");
  cr_assert_eq(func->as.function.param_count, 2, "should have 2 parameters");
  cr_assert_str_eq(func->as.function.params[0], "x", "first param should be 'x'");
  cr_assert_str_eq(func->as.function.params[1], "y", "second param should be 'y'");
  cr_assert_not_null(func->as.function.body, "function body should not be NULL");
  cr_assert_eq(func->as.function.body->type, L_SYMBOL, "body should be a symbol");

  lval_free(func);
  symbol_intern_free_all();
}

Test(lval_tests, function_copy_works) {
  symbol_intern_init();

  char **params = malloc(1 * sizeof(char *));
  params[0] = strdup("x");
  lval_t *body = lval_num(42);
  lval_t *original = lval_function(params, 1, body, NULL);

  lval_t *copy = lval_copy(original);

  cr_assert_not_null(copy, "copied function should not be NULL");
  cr_assert_eq(copy->type, L_FUNCTION, "copy should be L_FUNCTION");
  cr_assert_eq(copy->as.function.param_count, 1, "copy should have 1 parameter");
  cr_assert_str_eq(copy->as.function.params[0], "x", "copy param should be 'x'");
  cr_assert_neq(copy->as.function.params,
                original->as.function.params,
                "param arrays should be different pointers");

  lval_free(original);
  lval_free(copy);
  symbol_intern_free_all();
}

Test(lval_tests, prints_all_atoms, .init = redirect_stdout) {
  symbol_intern_init();

  lval_t *v_num = lval_num(42);
  lval_t *v_sym = lval_intern("test_symbol");
  lval_t *v_str = lval_string_copy("test_string", 11);
  lval_t *v_nil = lval_nil();
  lval_t *v_true = lval_bool(true);
  lval_t *v_false = lval_bool(false);

  lval_print(v_num);
  putchar('\n');
  lval_print(v_sym);
  putchar('\n');
  lval_print(v_str);
  putchar('\n');
  lval_print(v_nil);
  putchar('\n');
  lval_print(v_true);
  putchar('\n');
  lval_print(v_false);
  putchar('\n');

  fflush(stdout);

  cr_assert_stdout_eq_str("42\n"
                          "test_symbol\n"
                          "\"test_string\"\n"
                          "nil\n"
                          "true\n"
                          "false\n");

  lval_free(v_num);
  lval_free(v_sym);
  lval_free(v_str);
  lval_free(v_nil);
  lval_free(v_true);
  lval_free(v_false);

  symbol_intern_free_all();
}

Test(lval_tests, prints_function, .init = redirect_stdout) {
  symbol_intern_init();

  char **params = malloc(1 * sizeof(char *));
  params[0] = strdup("x");
  lval_t *body = lval_num(42);
  lval_t *func = lval_function(params, 1, body, NULL);

  lval_print(func);
  putchar('\n');
  fflush(stdout);

  cr_assert_stdout_eq_str("<function>\n");

  lval_free(func);
  symbol_intern_free_all();
}

Test(lval_tests, it_prints_lists, .init = redirect_stdout) {
  symbol_intern_init();

  lval_t *pair = lval_cons(lval_num(1), lval_num(2));

  lval_t *list = lval_cons(lval_num(1), lval_cons(lval_num(2), lval_cons(lval_num(3), lval_nil())));

  lval_t *single = lval_cons(lval_intern("hello"), lval_nil());

  lval_t *mixed = lval_cons(
      lval_num(42), lval_cons(lval_string_copy("test", 4), lval_cons(lval_bool(true), lval_nil())));

  lval_t *empty = lval_nil();

  lval_print(pair);
  putchar('\n');
  lval_print(list);
  putchar('\n');
  lval_print(single);
  putchar('\n');
  lval_print(mixed);
  putchar('\n');
  lval_print(empty);
  putchar('\n');

  fflush(stdout);

  cr_assert_stdout_eq_str("(1 . 2)\n"
                          "(1 . (2 . (3 . nil)))\n"
                          "(hello . nil)\n"
                          "(42 . (\"test\" . (true . nil)))\n"
                          "nil\n");

  lval_free(pair);
  lval_free(list);
  lval_free(single);
  lval_free(mixed);
  lval_free(empty);

  symbol_intern_free_all();
}
