#include "builtin.h"
#include "env.h"
#include "evaluator.h"
#include "lexer.h"
#include "lval.h"
#include "parser.h"
#include "symbol.h"
#include <criterion/criterion.h>
#include <math.h>
#include <stdbool.h>

static parse_result_t setup_input(const char *input, parser_t *out_parser) {
  lexer_t lexer = lexer_new(input);
  *out_parser = parser_new(&lexer);
  parse_result_t result = parser_parse(out_parser);
  if (out_parser->error_count > 0) {
    for (size_t i = 0; i < out_parser->error_count; i++) {
      fprintf(stderr, "Parser error: %s\n", out_parser->errors[i]);
    }
  }
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

Test(define_tests, define_binds_number_and_lookup_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define x 42) x", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(is_num(r2.result, 42.0));
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(define_tests, define_rhs_is_evaluated_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define y 5) (define z y) z", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(is_num(r3.result, 5.0));
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(define_tests, define_errors_when_name_not_symbol_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define 1 2)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert(r.error_message != NULL);
  evaluator_result_free(&r);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(define_tests, define_errors_on_wrong_arity_no_args_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(define_tests, define_errors_on_wrong_arity_one_arg_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define x)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(define_tests, define_errors_on_wrong_arity_too_many_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define x 1 2)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(define_tests, define_errors_when_rhs_fails_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define a b)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert(r.error_message != NULL);
  evaluator_result_free(&r);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(define_tests, define_binds_quoted_list_single_parser) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define l '(1 2)) l", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);

  lval_t *lst = r2.result;
  require_cons(lst);
  cr_assert(is_num(car(lst), 1.0));
  lval_t *rest = cdr(lst);
  require_cons(rest);
  cr_assert(is_num(car(rest), 2.0));
  cr_assert_eq(cdr(rest)->type, L_NIL);

  evaluator_result_free(&r2);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, set_errors_on_unbound) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(set x 1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, set_top_level_updates_and_returns_value) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define x 1) (set x 2) x", &p);
  eval_result_t r = evaluate_many(pr.expressions, pr.count, &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 2.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, set_mutates_outer_env_from_lambda) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  /* (define x 1)
     (define bump (lambda () (set x (+ x 1))))
     (bump) (bump) x  ==> 3 */
  parse_result_t pr = setup_input("(define x 1)"
                                  "(define bump (lambda () (set x (+ x 1))))"
                                  "(bump)"
                                  "(bump)"
                                  "x",
                                  &p);
  eval_result_t r = evaluate_many(pr.expressions, pr.count, &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 3.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, set_returns_assigned_value) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define x 10) (equal (set x 42) 42)", &p);
  eval_result_t r = evaluate_many(pr.expressions, pr.count, &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert(r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, set_can_rebind_builtin_in_global_env) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  /* Replace + with subtraction and verify */
  parse_result_t pr = setup_input("(set + (lambda (a b) (- a b)))"
                                  "(+ 5 2)",
                                  &p);
  eval_result_t r = evaluate_many(pr.expressions, pr.count, &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 3.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, if_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(list (if #t 1 2) (if #f 1 2))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  lval_t *a = r.result->as.cons.car;
  lval_t *b = r.result->as.cons.cdr->as.cons.car;
  cr_assert_eq(a->type, L_NUM);
  cr_assert_float_eq(a->as.number, 1.0, 1e-10);
  cr_assert_eq(b->type, L_NUM);
  cr_assert_float_eq(b->as.number, 2.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, if_no_else_returns_nil) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(list (if #t 1) (if #f 1))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  lval_t *a = r.result->as.cons.car;
  lval_t *b = r.result->as.cons.cdr->as.cons.car;
  cr_assert_eq(a->type, L_NUM);
  cr_assert_eq(b->type, L_NIL);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, if_condition_must_be_bool) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(if 1 'a 'b)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, if_is_lazy) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr =
      setup_input("(list (if #t 1 (error \"boom\")) (if #f (error \"boom\") 2))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  lval_t *a = r.result->as.cons.car;
  lval_t *b = r.result->as.cons.cdr->as.cons.car;
  cr_assert_eq(a->type, L_NUM);
  cr_assert_float_eq(a->as.number, 1.0, 1e-10);
  cr_assert_eq(b->type, L_NUM);
  cr_assert_float_eq(b->as.number, 2.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, cond_basic_first_true) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(equal (cond (#f 'no #t 'yes #t 'later)) 'yes)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert(r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, cond_none_true_returns_nil) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(cond (#f 1 #f 2))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NIL);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, cond_nonboolean_condition_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(cond (1 'a #t 'b))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, cond_odd_length_list_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(cond (#t 1 #f))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, cond_argument_must_be_list) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(cond 42)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, cond_dotted_list_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(cond (1 . 2))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, cond_is_lazy) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr =
      setup_input("(equal (cond (#f (error \"boom\") #t 7 #t (error \"boom2\"))) 7)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert(r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, begin_returns_last_and_evaluates_in_order) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin 1 2 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 3.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, begin_uses_current_env) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin (define x 1) (set x 2) x)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 2.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, begin_zero_forms_returns_nil) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NIL);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(special_forms, quote_does_not_evaluate_unquote) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(equal (quote (unquote (+ 1 2))) '(unquote (+ 1 2)))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert(r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quasiquote, basic_literal_list) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(equal `(1 2 3) '(1 2 3))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(r.result->type == L_BOOL && r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quasiquote, unquote_values) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr =
      setup_input("(begin (define x 10) (equal `(a ,x b ,(+ 1 2)) '(a 10 b 3)))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(r.result->type == L_BOOL && r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quasiquote, splicing_basic_and_empty) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin (define xs '(2 3))"
                                  "       (define ys '())"
                                  "       (list (equal `(1 ,@xs 4) '(1 2 3 4))"
                                  "             (equal `(1 ,@ys 4) '(1 4))))",
                                  &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  lval_t *a = r.result->as.cons.car;
  lval_t *b = r.result->as.cons.cdr->as.cons.car;
  cr_assert(a->type == L_BOOL && a->as.boolean);
  cr_assert(b->type == L_BOOL && b->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quasiquote, dotted_tail_unquote) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin (define tail '(2 3)) (equal `(1 . ,tail) '(1 2 3)))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(r.result->type == L_BOOL && r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quasiquote, splice_in_tail_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin (define xs '(1 2)) `(1 . ,@xs))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(quasiquote, nested_quasiquote_is_data) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(equal `(quasiquote (1 ,x)) '(quasiquote (1 (unquote x))))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  if (r.status != EVAL_OK) {
    fprintf(stderr, "Error: %s\n", r.error_message);
  }
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(r.result->type == L_BOOL);
  cr_assert(r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(macros, defmacro_defines_symbol) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(defmacro m (x) x)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_SYMBOL);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(macros, identity_macro_returns_argument_code) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin "
                                  "  (defmacro id (x) x) "
                                  "  (equal (id 42) 42))",
                                  &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(r.result->type == L_BOOL && r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(macros, constant_macro) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin "
                                  "  (defmacro k0 () '99) "
                                  "  (= (k0) 99))",
                                  &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(r.result->type == L_BOOL && r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(macros, twice_quasiquote) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(begin "
                                  "  (defmacro twice (x) `( + ,x ,x)) "
                                  "  (= (twice 5) 10))",
                                  &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(r.result->type == L_BOOL && r.result->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(macros, macro_unless) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  env_add_builtins(&env);
  parser_t p = (parser_t){ 0 };
  parse_result_t pr =
      setup_input("(begin "
                  "  (defmacro unless (cond then else) `(if (not ,cond) ,then ,else)) "
                  "  (list (equal (unless #f 1 2) 1) "
                  "        (equal (unless #t 1 2) 2)))",
                  &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  lval_t *a = r.result->as.cons.car;
  lval_t *b = r.result->as.cons.cdr->as.cons.car;
  cr_assert(a->type == L_BOOL && a->as.boolean);
  cr_assert(b->type == L_BOOL && b->as.boolean);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}
