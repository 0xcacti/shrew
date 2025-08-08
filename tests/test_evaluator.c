#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include <criterion/criterion.h>

s_expression_t **setup_input(const char *input, parser_t *out_parser) {
  lexer_t lexer = lexer_new(input);
  *out_parser = parser_new(&lexer);
  parse_result_t result = parser_parse(out_parser);
  cr_assert_eq(out_parser->error_count, 0, "Parser should have no errors");
  cr_assert_not_null(result.expressions, "Parsed expressions should not be NULL");
  return result.expressions;
}

// Test(evaluator_tests, evaluate_atoms) {
//   const char *input = "42";
//
//   env_t env = env_new();
//   s_expression_t **expressions = setup_input(input);
//   s_expression_t *expr = expressions[0];
//
//   eval_result_t res = evaluate_single(expr, &env);
//   cr_assert_eq(res.status, EVAL_OK, "Evaluation should succeed");
//   cr_assert_eq(res.result, expr, "Numbers should evaluate to themselves");
//   evaluator_result_free(&res);
//   parse_result_free();
// }
