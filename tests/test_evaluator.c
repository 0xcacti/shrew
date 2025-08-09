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

Test(evaluator_tests, evaluate_number_atom) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("42", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert_eq(res.result->type, L_NUM);
  cr_assert(fabs(res.result->as.number - 42.0) < 1e-9);

  lval_free(res.result);
  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_tests, evaluate_string_atom) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("\"Hello, World!\"", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert_eq(res.result->type, L_STRING);
  cr_assert_str_eq(res.result->as.string.ptr, "Hello, World!");

  lval_free(res.result);
  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_tests, evaluate_boolean_atom) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("#t #f", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert_eq(res.result->type, L_BOOL);
  cr_assert(res.result->as.boolean);

  expr = pr.expressions[1];
  res = evaluate_single(expr, &env);
  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert_eq(res.result->type, L_BOOL);
  cr_assert(!res.result->as.boolean);

  lval_free(res.result);
  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_tests, evaluate_predefined_symbol) {
  symbol_intern_init();

  env_t env;
  cr_assert(env_init(&env, NULL));
  bool success = env_define(&env, "meow", lval_num(42));
  cr_assert(success, "Failed to define 'meow' in the environment");

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("meow", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t res = evaluate_single(expr, &env);

  cr_assert_eq(res.status, EVAL_OK);
  cr_assert_not_null(res.result);
  cr_assert_eq(res.result->type, L_NUM);
  cr_assert(fabs(res.result->as.number - 42.0) < 1e-9);

  lval_free(res.result);
  evaluator_result_free(&res);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_tests, evaluate_unbound_symbol_errors) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("does-not-exist", &parser);

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

Test(evaluator_tests, evaluate_symbol_env_chain_and_shadowing) {
  symbol_intern_init();

  env_t parent;
  cr_assert(env_init(&parent, NULL));
  env_t child;
  cr_assert(env_init(&child, &parent));

  cr_assert(env_define(&parent, "x", lval_num(1)));
  {
    parser_t p = { 0 };
    parse_result_t pr = setup_input("x", &p);
    eval_result_t r = evaluate_single(pr.expressions[0], &child);
    cr_assert_eq(r.status, EVAL_OK);
    cr_assert_eq(r.result->type, L_NUM);
    cr_assert(fabs(r.result->as.number - 1.0) < 1e-9);
    lval_free(r.result);
    evaluator_result_free(&r);
    parse_result_free(&pr);
    parser_free(&p);
  }

  cr_assert(env_define(&child, "x", lval_num(2)));
  {
    parser_t p = { 0 };
    parse_result_t pr = setup_input("x", &p);
    eval_result_t r = evaluate_single(pr.expressions[0], &child);
    cr_assert_eq(r.status, EVAL_OK);
    cr_assert_eq(r.result->type, L_NUM);
    cr_assert(fabs(r.result->as.number - 2.0) < 1e-9);
    lval_free(r.result);
    evaluator_result_free(&r);
    parse_result_free(&pr);
    parser_free(&p);
  }

  env_destroy(&child);
  env_destroy(&parent);
  symbol_intern_free_all();
}

Test(evaluator_tests, evaluate_literal_allocates_fresh_lval_each_time) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("7", &parser);

  s_expression_t *expr = pr.expressions[0];
  eval_result_t r1 = evaluate_single(expr, &env);
  eval_result_t r2 = evaluate_single(expr, &env);

  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert_neq(r1.result, r2.result, "Expected distinct lval allocations per eval()");
  cr_assert_eq(r1.result->type, L_NUM);
  cr_assert_eq(r2.result->type, L_NUM);
  cr_assert(fabs(r1.result->as.number - 7.0) < 1e-9);
  cr_assert(fabs(r2.result->as.number - 7.0) < 1e-9);

  lval_free(r1.result);
  lval_free(r2.result);
  evaluator_result_free(&r1);
  evaluator_result_free(&r2);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}

Test(evaluator_tests, evaluate_null_expr_is_error) {
  env_t env;
  cr_assert(env_init(&env, NULL));
  eval_result_t r = evaluate_single(NULL, &env);
  cr_assert_eq(r.status, EVAL_ERR);
  cr_assert_not_null(r.error_message);
  evaluator_result_free(&r);
  env_destroy(&env);
}

Test(evaluator_tests, evaluate_string_allocates_fresh_copy_each_time) {
  symbol_intern_init();
  env_t env;
  cr_assert(env_init(&env, NULL));

  parser_t parser = { 0 };
  parse_result_t pr = setup_input("\"abc\"", &parser);

  eval_result_t r1 = evaluate_single(pr.expressions[0], &env);
  eval_result_t r2 = evaluate_single(pr.expressions[0], &env);

  cr_assert_eq(r1.status, EVAL_OK);
  cr_assert_eq(r2.status, EVAL_OK);
  cr_assert_eq(r1.result->type, L_STRING);
  cr_assert_eq(r2.result->type, L_STRING);
  cr_assert_str_eq(r1.result->as.string.ptr, "abc");
  cr_assert_str_eq(r2.result->as.string.ptr, "abc");
  cr_assert_neq(r1.result->as.string.ptr,
                r2.result->as.string.ptr,
                "Distinct heap buffers expected for each string literal evaluation");

  lval_free(r1.result);
  lval_free(r2.result);
  evaluator_result_free(&r1);
  evaluator_result_free(&r2);
  parse_result_free(&pr);
  parser_free(&parser);
  env_destroy(&env);
  symbol_intern_free_all();
}
