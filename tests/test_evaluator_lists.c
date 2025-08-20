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

Test(evaluator_lists, evaluates_user_defined_functions_simple) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define add2 (lambda (x) (+ x 2)))"
                                  " (add2 14)",
                                  &parser);

  s_expression_t *def_expr = pr.expressions[0];
  eval_result_t def_res = evaluate_single(def_expr, &env);
  cr_assert_eq(def_res.status, EVAL_OK);
  evaluator_result_free(&def_res);

  s_expression_t *call_expr = pr.expressions[1];
  eval_result_t call_res = evaluate_single(call_expr, &env);
  if (call_res.status != EVAL_OK) {
    fprintf(stderr, "Error evaluating call: %s\n", call_res.error_message);
  }
  cr_assert_eq(call_res.status, EVAL_OK);
  cr_assert_not_null(call_res.result);
  cr_assert(is_num(call_res.result, 16.0));

  lval_free(call_res.result);
  evaluator_result_free(&call_res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_lists, evaluates_user_defined_functions_closure) {
  symbol_intern_init();

  env_t *env = malloc(sizeof(env_t));
  cr_assert(env_init(env, NULL));

  parser_t parser = (parser_t){ 0 };
  parse_result_t pr = setup_input("(define make-adder (lambda (a) (lambda (x) (+ x a))))"
                                  " (define add7 (make-adder 7))"
                                  " (add7 9)",
                                  &parser);

  eval_result_t r0 = evaluate_single(pr.expressions[0], env);
  cr_assert_eq(r0.status, EVAL_OK);
  evaluator_result_free(&r0);

  eval_result_t r1 = evaluate_single(pr.expressions[1], env);
  cr_assert_eq(r1.status, EVAL_OK);
  evaluator_result_free(&r1);

  eval_result_t r2 = evaluate_single(pr.expressions[2], env);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert_not_null(r2.result);
  cr_assert(is_num(r2.result, 16.0));

  lval_free(r2.result);
  evaluator_result_free(&r2);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(env);
  symbol_intern_free_all();
}
