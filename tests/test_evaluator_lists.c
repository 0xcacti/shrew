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
