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

Test(mod_tests, mod_two_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(mod 10 3)", &p);
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

Test(mod_tests, mod_zero_divisor_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(mod 10 0)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(mod_tests, mod_non_number_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(mod 10 #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(abs_tests, abs_positive_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(abs 5)", &p);
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

Test(abs_tests, abs_negative_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(abs -3.5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert(is_num(r.result, 3.5));
  lval_free(r.result);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(abs_tests, abs_zero) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(abs 0)", &p);
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

Test(abs_tests, abs_non_number_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(abs #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(abs_tests, abs_no_args_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(abs)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);

  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(abs_tests, abs_multiple_args_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = { 0 };
  parse_result_t pr = setup_input("(abs 1 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, min_single_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(min 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 5.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, min_multiple_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(min 5 2 8 1 9)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 1.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, min_negative_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(min -5 -2 -8)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, -8.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, min_no_args_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(min)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, min_non_number_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(min 5 #t 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, max_single_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(max 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 5.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, max_multiple_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(max 5 2 8 1 9)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 9.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(min_max_tests, max_negative_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(max -5 -2 -8)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, -2.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, floor_positive_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(floor 3.7)", &p);
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

Test(rounding_tests, floor_negative_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(floor -3.7)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, -4.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, floor_whole_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(floor 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 5.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, floor_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(floor 1 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, floor_non_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(floor #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, ceil_positive_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(ceil 3.2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 4.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, ceil_negative_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(ceil -3.2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, -3.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, ceil_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(ceil)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, round_positive_half_up) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(round 3.5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 4.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, round_negative_half_down) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(round -3.5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, -4.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, round_low_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(round 3.2)", &p);
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

Test(rounding_tests, trunc_positive_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(trunc 3.9)", &p);
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

Test(rounding_tests, trunc_negative_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(trunc -3.9)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, -3.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(rounding_tests, trunc_non_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(trunc \"hello\")", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, exp_zero) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(exp 0)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 1.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, exp_one) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(exp 1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 2.718281828, 1e-6);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, exp_negative) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(exp -1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 0.367879441, 1e-6);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, exp_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(exp 1 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, exp_non_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(exp #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, log_one) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(log 1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 0.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, log_e) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(log 2.718281828)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 1.0, 1e-6);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, log_ten) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(log 10)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 2.302585093, 1e-6);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, log_zero_domain_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(log 0)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, log_negative_nan) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(log -1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, log_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(log)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, sqrt_perfect_squares) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(sqrt 16)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 4.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, sqrt_zero) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(sqrt 0)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 0.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, sqrt_decimal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(sqrt 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 1.414213562, 1e-6);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, sqrt_negative_nan) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(sqrt -1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(math_tests, sqrt_non_number) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(sqrt \"hello\")", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, eq_two_equal_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(= 5 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, eq_two_unequal_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(= 5 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, eq_multiple_equal_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(= 3 3 3 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, eq_multiple_with_one_different) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(= 3 3 5 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, eq_one_arg_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(= 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, eq_non_number_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(= 5 #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, lt_two_ascending) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(< 3 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, lt_two_descending) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(< 5 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, lt_multiple_ascending) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(< 1 2 3 4)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, lt_multiple_with_equal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(< 1 2 2 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, lt_one_arg_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(< 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, gt_two_descending) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(> 5 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, gt_multiple_descending) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(> 5 4 3 1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, gt_multiple_not_monotonic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(> 5 3 4 1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, le_equal_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(<= 3 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, le_ascending_with_equal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(<= 1 2 2 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, le_violates_order) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(<= 3 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, ge_equal_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(>= 5 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, ge_descending_with_equal) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(>= 5 4 4 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(comparison_tests, ge_violates_order) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(>= 2 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_same_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq 5 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_different_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq 5 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_same_booleans) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq #t #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_different_booleans) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq #t #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_same_symbols) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq 'hello 'hello)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_different_symbols) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq 'hello 'world)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_different_types) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq 5 #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, eq_too_many_args) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(eq 5 5 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_same_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal 5 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_different_numbers) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal 5 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_same_strings) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal \"hello\" \"hello\")", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_different_strings) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal \"hello\" \"world\")", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_simple_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal '(1 2 3) '(1 2 3))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_different_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal '(1 2 3) '(1 2 4))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_nested_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal '(1 (2 3) 4) '(1 (2 3) 4))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_different_nested_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal '(1 (2 3) 4) '(1 (2 5) 4))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_empty_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal '() '())", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_different_length_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal '(1 2) '(1 2 3))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_mixed_types_in_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal '(1 \"hello\" #t) '(1 \"hello\" #t))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_different_types) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal 5 \"5\")", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(equality_tests, equal_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(equal 5 5 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, boolean_not_fails_on_non_boolean) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(not 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, not_true_becomes_false) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(not #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, not_false_becomes_true) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(not #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, not_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(not #t #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, not_no_args_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(not)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, and_all_true) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(and #t #t #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, and_one_false) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(and #t #f #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, and_all_false) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(and #f #f #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, and_single_true) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(and #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, and_single_false) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(and #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, and_no_args_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(and)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, and_non_boolean_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(and #t 5 #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, or_all_false) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(or #f #f #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, or_one_true) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(or #f #t #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, or_all_true) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(or #t #t #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, or_single_true) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(or #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, true);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, or_single_false) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(or #f)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_BOOL);
  cr_assert_eq(r.result->as.boolean, false);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, or_no_args_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(or)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(boolean_tests, or_non_boolean_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(or #f \"hello\" #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

// CONS tests
Test(list_tests, cons_creates_pair) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(cons 1 2)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_CONS);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, cons_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(cons 1)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

// CAR tests
Test(list_tests, car_gets_first_element) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(car '(1 2 3))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 1.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, car_empty_list_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(car '())", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, car_non_cons_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(car 5)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

// CDR tests
Test(list_tests, cdr_gets_rest) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(cdr '(1 2 3))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_CONS);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, cdr_non_cons_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(cdr \"hello\")", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

// LIST tests
Test(list_tests, list_creates_list) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(list 1 2 3)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_CONS);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, list_empty_creates_nil) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(list)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

// LENGTH tests
Test(list_tests, length_counts_elements) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(length '(1 2 3 4))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 4.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, length_empty_list) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(length '())", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 0.0, 1e-10);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, length_non_cons_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(length 42)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

// APPEND tests
Test(list_tests, append_combines_lists) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(append '(1 2) '(3 4))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_CONS);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, append_wrong_arg_count) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(append '(1 2))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

// REVERSE tests
Test(list_tests, reverse_reverses_list) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(reverse '(1 2 3))", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_CONS);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, reverse_empty_list) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(reverse '())", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(list_tests, reverse_non_cons_error) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = { 0 };
  parse_result_t pr = setup_input("(reverse #t)", &p);
  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, atom_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr =
      setup_input("(atom? 1) (atom? '()) (atom? '(1)) (atom? \"hi\") (atom? 'x)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert_eq(r1.result->type, L_BOOL);
  cr_assert(r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert_eq(r2.result->type, L_BOOL);
  cr_assert(!r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert_eq(r3.result->type, L_BOOL);
  cr_assert(!r3.result->as.boolean);
  evaluator_result_free(&r3);

  eval_result_t r4 = evaluate_single(pr.expressions[3], &env);
  cr_assert_eq(r4.status, EVAL_OK);
  cr_assert_eq(r4.result->type, L_BOOL);
  cr_assert(r4.result->as.boolean);
  evaluator_result_free(&r4);

  eval_result_t r5 = evaluate_single(pr.expressions[4], &env);
  cr_assert_eq(r5.status, EVAL_OK);
  cr_assert_eq(r5.result->type, L_BOOL);
  cr_assert(r5.result->as.boolean);
  evaluator_result_free(&r5);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, atom_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(atom?) (atom? 1 2)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, list_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(list? '()) (list? '(1 2)) (list? 1)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(r1.result->type == L_BOOL && r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(r2.result->type == L_BOOL && r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(r3.result->type == L_BOOL && !r3.result->as.boolean);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, list_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(list?) (list? 1 2)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, null_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(null? '()) (null? '(1)) (null? 1)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(r1.result->type == L_BOOL && r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(r2.result->type == L_BOOL && !r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(r3.result->type == L_BOOL && !r3.result->as.boolean);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, null_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(null?) (null? 1 2)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, number_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(number? 3.14) (number? 'x) (number? '())", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(r1.result->type == L_BOOL && r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(r2.result->type == L_BOOL && !r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(r3.result->type == L_BOOL && !r3.result->as.boolean);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, number_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(number?) (number? 1 2)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, symbol_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(symbol? 'x) (symbol? 1) (symbol? \"hi\")", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(r1.result->type == L_BOOL && r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(r2.result->type == L_BOOL && !r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(r3.result->type == L_BOOL && !r3.result->as.boolean);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, symbol_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(symbol?) (symbol? 'x 'y)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, string_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(string? \"hi\") (string? 1) (string? 'x)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(r1.result->type == L_BOOL && r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(r2.result->type == L_BOOL && !r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(r3.result->type == L_BOOL && !r3.result->as.boolean);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, string_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(string?) (string? \"a\" \"b\")", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, pair_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(pair? '(1 . 2)) (pair? '(1)) (pair? '()) (pair? 1)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(r1.result->type == L_BOOL && r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(r2.result->type == L_BOOL && r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(r3.result->type == L_BOOL && !r3.result->as.boolean);
  evaluator_result_free(&r3);

  eval_result_t r4 = evaluate_single(pr.expressions[3], &env);
  cr_assert_eq(r4.status, EVAL_OK);
  cr_assert(r4.result->type == L_BOOL && !r4.result->as.boolean);
  evaluator_result_free(&r4);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, pair_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(pair?) (pair? 1 2)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, function_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(function? 'number?) (function? 'x) (function? 1)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(r1.result->type == L_BOOL && r1.result->as.boolean);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(r2.result->type == L_BOOL && !r2.result->as.boolean);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(r3.result->type == L_BOOL && !r3.result->as.boolean);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(type_predicates, function_arity_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));
  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(function?) (function? 'x 'y)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(string_builtins, string_length_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input(
      "(string-length \"\") (string-length \"abc\") (string-length \"hello world\")", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(is_num(r1.result, 0.0));
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(is_num(r2.result, 3.0));
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(is_num(r3.result, 11.0));
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(string_builtins, string_length_arity_and_type_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr =
      setup_input("(string-length) (string-length \"a\" \"b\") (string-length 1)", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_ERR);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(string_builtins, string_append_zero_one_many) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(string-append)"
                                  "(string-append \"hi\")"
                                  "(string-append \"a\" \"\" \"bc\" \"d\")",
                                  &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert_eq(r1.result->type, L_STRING);
  cr_assert_str_eq(r1.result->as.string.ptr, "");
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert_eq(r2.result->type, L_STRING);
  cr_assert_str_eq(r2.result->as.string.ptr, "hi");
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert_eq(r3.result->type, L_STRING);
  cr_assert_str_eq(r3.result->as.string.ptr, "abcd");
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(string_builtins, string_append_type_error_any_non_string_arg_fails) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(string-append 1)"
                                  "(string-append \"a\" 'x)"
                                  "(string-append \"a\" \"b\" 3)",
                                  &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_ERR);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(cast_builtins, number_to_string_roundtrip) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(string->number (number->string 0))"
                                  "(string->number (number->string -0))"
                                  "(string->number (number->string 1))"
                                  "(string->number (number->string -123.5))"
                                  "(string->number (number->string 3.141592653589793))",
                                  &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(is_num(r1.result, 0.0));
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(is_num(r2.result, 0.0));
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(is_num(r3.result, 1.0));
  evaluator_result_free(&r3);

  eval_result_t r4 = evaluate_single(pr.expressions[3], &env);
  cr_assert_eq(r4.status, EVAL_OK);
  cr_assert(is_num(r4.result, -123.5));
  evaluator_result_free(&r4);

  eval_result_t r5 = evaluate_single(pr.expressions[4], &env);
  cr_assert_eq(r5.status, EVAL_OK);
  cr_assert(is_num(r5.result, 3.141592653589793));
  evaluator_result_free(&r5);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(cast_builtins, number_to_string_arity_and_type_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr =
      setup_input("(number->string) (number->string 1 2) (number->string \"a\")", &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_ERR);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_ERR);
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_ERR);
  evaluator_result_free(&r3);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(cast_builtins, string_to_number_success_and_failures) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(string->number \"0\")"
                                  "(string->number \"  +2.5e1 \")"
                                  "(string->number \"-3.14\")"
                                  "(string->number \"\")"
                                  "(string->number \"abc\")"
                                  "(string->number 1)",
                                  &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert(is_num(r1.result, 0.0));
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert(is_num(r2.result, 25.0));
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert(is_num(r3.result, -3.14));
  evaluator_result_free(&r3);

  eval_result_t r4 = evaluate_single(pr.expressions[3], &env);
  cr_assert_eq(r4.status, EVAL_ERR);
  evaluator_result_free(&r4);

  eval_result_t r5 = evaluate_single(pr.expressions[4], &env);
  cr_assert_eq(r5.status, EVAL_ERR);
  evaluator_result_free(&r5);

  eval_result_t r6 = evaluate_single(pr.expressions[5], &env);
  cr_assert_eq(r6.status, EVAL_ERR);
  evaluator_result_free(&r6);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(cast_builtins, symbol_string_roundtrip_and_types) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(symbol->string 'foo)"
                                  "(string->symbol \"foo\")"
                                  "(string->symbol \"a-b?+\")"
                                  "(symbol->string 1)"
                                  "(string->symbol 1)"
                                  "(symbol->string)"
                                  "(string->symbol \"x\" \"y\")",
                                  &p);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert_eq(r1.result->type, L_STRING);
  cr_assert_str_eq(r1.result->as.string.ptr, "foo");
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[1], &env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert_eq(r2.result->type, L_SYMBOL);
  cr_assert_str_eq(r2.result->as.symbol.name, "foo");
  evaluator_result_free(&r2);

  eval_result_t r3 = evaluate_single(pr.expressions[2], &env);
  cr_assert_eq(r3.status, EVAL_OK);
  cr_assert_eq(r3.result->type, L_SYMBOL);
  cr_assert_str_eq(r3.result->as.symbol.name, "a-b?+");
  evaluator_result_free(&r3);

  eval_result_t r4 = evaluate_single(pr.expressions[3], &env);
  cr_assert_eq(r4.status, EVAL_ERR);
  evaluator_result_free(&r4);

  eval_result_t r5 = evaluate_single(pr.expressions[4], &env);
  cr_assert_eq(r5.status, EVAL_ERR);
  evaluator_result_free(&r5);

  eval_result_t r6 = evaluate_single(pr.expressions[5], &env);
  cr_assert_eq(r6.status, EVAL_ERR);
  evaluator_result_free(&r6);

  eval_result_t r7 = evaluate_single(pr.expressions[6], &env);
  cr_assert_eq(r7.status, EVAL_ERR);
  evaluator_result_free(&r7);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(functional_builtins, apply_basic) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t p = (parser_t){ 0 };
  parse_result_t pr = setup_input("(apply + '(1 2 3))", &p);

  eval_result_t r = evaluate_single(pr.expressions[0], &env);
  cr_assert_eq(r.status, EVAL_OK);
  cr_assert_eq(r.result->type, L_NUM);
  cr_assert_float_eq(r.result->as.number, 6.0, 1e-10);
  evaluator_result_free(&r);

  parse_result_free(&pr);
  parser_free(&p);
  env_destroy(&env);
  symbol_intern_free_all();
}
