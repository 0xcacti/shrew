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

Test(evaluator_test, evaluate_empty_list) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("()", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert_eq(res.result->type, L_NIL);

  lval_free(res.result);
  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_lists, dotted_list_call_errors) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("(+ 1 . 2)", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_ERR);
  cr_assert_not_null(res.error_message);
  cr_assert(strstr(res.error_message, "Dotted list") != NULL);

  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_lists, head_is_number_errors) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("(1 2 3)", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_ERR);
  cr_assert_not_null(res.error_message);
  cr_assert(strstr(res.error_message, "Expected a function, got: 0") == 0);

  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_lists, head_is_list_errors) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("((meow-fn 1 2) 3)", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_ERR);
  cr_assert_not_null(res.error_message);
  cr_assert(strstr(res.error_message, "Unknown function") != NULL);

  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_lists, unknown_function_symbol_errors) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("(does-not-exist 1)", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_ERR);
  cr_assert_not_null(res.error_message);
  cr_assert(strstr(res.error_message, "does-not-exist") != NULL);

  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_lists, nested_calls_evaluate_arguments) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("(+ 1 (* 2 3) (- 10 4) (/ 9 3))", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert(is_num(res.result, 16.0));

  lval_free(res.result);
  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_lists, evaluates_user_defined_functions) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("(define ", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert(is_num(res.result, 16.0));

  lval_free(res.result);
  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}
