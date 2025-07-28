#include "parser.h"
#include <criterion/criterion.h>

Test(parser_tests, it_parses_numbers) {
  const char *input = "123";
  lexer_t lexer = lexer_new(input);
  parser_t parser = parser_new(&lexer);
  s_expression_t **sexp = parser_parse(&parser);
  cr_assert_not_null(sexp, "s_expression should not be NULL");
  cr_assert_eq(sexp[0]->type, NODE_ATOM,
               "first s_expression should be an atom");
  cr_assert_eq(sexp[0]->data.atom.type, ATOM_NUMBER,
               "atom type should be ATOM_NUMBER");
  cr_assert_eq(sexp[0]->data.atom.value.number, 123, "atom should be 123");
}
