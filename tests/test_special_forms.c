#include "env.h"
#include "evaluator.h"
#include "lexer.h"
#include "lval.h"
#include "parser.h"
#include "symbol.h"
#include <criterion/criterion.h>
#include <math.h>
#include <stdbool.h>

parse_result_t setup_input(const char *input, parser_t *out_parser) {
  lexer_t lexer = lexer_new(input);
  *out_parser = parser_new(&lexer);
  parse_result_t result = parser_parse(out_parser);
  cr_assert_eq(out_parser->error_count, 0, "Parser should have no errors");
  cr_assert_not_null(result.expressions, "Parsed expressions should not be NULL");
  return result;
}

static bool is_num(const lval_t *v, double x) {
  return v && v->type == L_NUM && fabs(v->as.number - x) < 1e-9;
}
static lval_t *car(lval_t *c) {
  cr_assert_eq(c->type, L_CONS);
  return c->as.cons.car;
}
static lval_t *cdr(lval_t *c) {
  cr_assert_eq(c->type, L_CONS);
  return c->as.cons.cdr;
}
static void require_cons(const lval_t *v) {
  cr_assert_eq(v->type, L_CONS);
}

Test(quote_tests, quote_symbol_returns_symbol_not_lookup) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  // Prove we don't evaluate: bind x=99; 'x should still be the SYMBOL "x"
  cr_assert(env_define(&env, "x", lval_num(99)));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'x", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_not_null(r.result);
  cr_assert_eq(r.result->type, L_SYMBOL);
  cr_assert_str_eq(r.result->as.symbol.name, "x");

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_number_string_boolean) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  // '42
  {
    parser_t p = { 0 };
    parse_result_t pr = setup_input("'42", &p);
    eval_result_t r = evaluate_single(pr.expressions[0], &env);
    cr_assert_eq(r.status, EVAL_OK);
    cr_assert(is_num(r.result, 42.0));
    lval_free(r.result);
    evaluator_result_free(&r);
    parse_result_free(&pr);
    parser_free(&p);
  }
  // '"hi"
  {
    parser_t p = { 0 };
    parse_result_t pr = setup_input("'\"hi\"", &p);
    eval_result_t r = evaluate_single(pr.expressions[0], &env);
    cr_assert_eq(r.status, EVAL_OK);
    cr_assert_eq(r.result->type, L_STRING);
    cr_assert_str_eq(r.result->as.string.ptr, "hi");
    lval_free(r.result);
    evaluator_result_free(&r);
    parse_result_free(&pr);
    parser_free(&p);
  }
  // '#t
  {
    parser_t p = { 0 };
    parse_result_t pr = setup_input("'#t", &p);
    eval_result_t r = evaluate_single(pr.expressions[0], &env);
    cr_assert_eq(r.status, EVAL_OK);
    cr_assert_eq(r.result->type, L_BOOL);
    cr_assert(r.result->as.boolean);
    lval_free(r.result);
    evaluator_result_free(&r);
    parse_result_free(&pr);
    parser_free(&p);
  }

  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_empty_list_is_nil) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'()", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NIL);

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_simple_list_builds_cons_chain) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'(1 2 3)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *a = r.result;
  require_cons(a);
  cr_assert(is_num(car(a), 1.0));

  lval_t *b = cdr(a);
  require_cons(b);
  cr_assert(is_num(car(b), 2.0));
  lval_t *c = cdr(b);
  require_cons(c);
  cr_assert(is_num(car(c), 3.0));
  cr_assert_eq(cdr(c)->type, L_NIL);

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_dotted_tail) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'(1 2 . 3)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *a = r.result;
  require_cons(a);
  cr_assert(is_num(car(a), 1.0));
  lval_t *b = cdr(a);
  require_cons(b);
  cr_assert(is_num(car(b), 2.0));
  cr_assert(is_num(b->as.cons.cdr, 3.0)); // dotted tail is number 3

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_nested_lists_and_mixed_atoms) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  // '((1 2) x "y" #f)
  parser_t p = { 0 };
  parse_result_t pr = setup_input("'((1 2) x \"y\" #f)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *lst = r.result;
  // first element: (1 2)
  require_cons(lst);
  lval_t *first = car(lst);
  require_cons(first);
  cr_assert(is_num(car(first), 1.0));
  require_cons(cdr(first));
  cr_assert(is_num(car(cdr(first)), 2.0));
  cr_assert_eq(cdr(cdr(first))->type, L_NIL);

  // second element: symbol x
  lval_t *rest = cdr(lst);
  require_cons(rest);
  cr_assert_eq(car(rest)->type, L_SYMBOL);
  cr_assert_str_eq(car(rest)->as.symbol.name, "x");

  // third element: "y"
  rest = cdr(rest);
  require_cons(rest);
  cr_assert_eq(car(rest)->type, L_STRING);
  cr_assert_str_eq(car(rest)->as.string.ptr, "y");

  // fourth element: #f
  rest = cdr(rest);
  require_cons(rest);
  cr_assert_eq(car(rest)->type, L_BOOL);
  cr_assert(!car(rest)->as.boolean);

  // end
  cr_assert_eq(cdr(rest)->type, L_NIL);

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_list_with_nil_elements) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  // '(() ())
  parser_t p = { 0 };
  parse_result_t pr = setup_input("'(() ())", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *l = r.result;
  require_cons(l);
  cr_assert_eq(car(l)->type, L_NIL);
  l = cdr(l);
  require_cons(l);
  cr_assert_eq(car(l)->type, L_NIL);
  cr_assert_eq(cdr(l)->type, L_NIL);

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_unquote_number_middle) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'(1 (unquote 5) 3)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *lst = r.result;
  require_cons(lst);
  cr_assert(is_num(car(lst), 1.0));

  lval_t *rest = cdr(lst);
  require_cons(rest);
  cr_assert(is_num(car(rest), 5.0)); /* (unquote 5) -> 5 */

  rest = cdr(rest);
  require_cons(rest);
  cr_assert(is_num(car(rest), 3.0));
  cr_assert_eq(cdr(rest)->type, L_NIL);

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_unquote_string_middle) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'(a (unquote \"hi\") b)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *lst = r.result;
  require_cons(lst);
  cr_assert_eq(car(lst)->type, L_SYMBOL);
  cr_assert_str_eq(car(lst)->as.symbol.name, "a");

  lval_t *rest = cdr(lst);
  require_cons(rest);
  cr_assert_eq(car(rest)->type, L_STRING);
  cr_assert_str_eq(car(rest)->as.string.ptr, "hi");

  rest = cdr(rest);
  require_cons(rest);
  cr_assert_eq(car(rest)->type, L_SYMBOL);
  cr_assert_str_eq(car(rest)->as.symbol.name, "b");
  cr_assert_eq(cdr(rest)->type, L_NIL);

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_unquote_in_head_position) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'((unquote 9) 2)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *lst = r.result;
  require_cons(lst);
  cr_assert(is_num(car(lst), 9.0)); /* first elem came from unquote */

  lval_t *rest = cdr(lst);
  require_cons(rest);
  cr_assert(is_num(car(rest), 2.0));
  cr_assert_eq(cdr(rest)->type, L_NIL);

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, quote_unquote_in_dotted_tail) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("'(1 2 . (unquote 3))", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);

  lval_t *a = r.result;
  require_cons(a);
  cr_assert(is_num(car(a), 1.0));

  lval_t *b = cdr(a);
  require_cons(b);
  cr_assert(is_num(car(b), 2.0));

  /* dotted tail evaluated from unquote */
  cr_assert(is_num(b->as.cons.cdr, 3.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quote_tests, unquote_top_level_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(unquote 1)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert(r.error_message != NULL);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}
