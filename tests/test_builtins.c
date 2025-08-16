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
  cr_assert_eq(out_parser->error_count, 0, "Parser should have no errors");
  cr_assert_not_null(result.expressions, "Parsed expressions should not be NULL");
  return result;
}

static bool is_num(const lval_t *v, double x) {
  return v && v->type == L_NUM && fabs(v->as.number - x) < 1e-9;
}

Test(add_tests, it_add_two_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(+ 1 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 3.0));
  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(add_tests, add_many_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(+ 1 2 3 4.5)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 10.5));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(add_tests, add_zero_args_returns_zero) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(+)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 0.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(add_tests, add_non_number_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(+ 1 #t)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(sub_tests, subtract_two_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(- 5 2)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 3.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(sub_tests, subtract_chain) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(- 10 1 2 3)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 4.0)); /* 10-1-2-3 = 4 */

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(sub_tests, subtract_unary_returns_same) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(- 7)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 7.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(sub_tests, subtract_zero_args_returns_zero) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(-)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 0.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(sub_tests, subtract_non_number_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(- 3 #f)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(mul_tests, multiply_two_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(* 6 7)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 42.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(mul_tests, multiply_chain) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(* 2 3 4)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 24.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(mul_tests, multiply_zero_args_returns_one) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(*)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 1.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(mul_tests, multiply_non_number_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(* 2 \"x\")", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(div_tests, divide_two_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(/ 6 3)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 2.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(div_tests, divide_chain) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(/ 20 2 2)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 5.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(div_tests, divide_unary_returns_same) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(/ 5)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 5.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(div_tests, divide_zero_args_returns_zero) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(/)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 0.0));

  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(div_tests, divide_non_number_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(/ 10 #t)", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}
