#include "parser.h"
#include <criterion/criterion.h>

Test(parser_tests, it_parses_numbers) {
  const char *input = "123\n0.134";
  lexer_t lexer = lexer_new(input);
  parser_t parser = parser_new(&lexer);
  s_expression_t **sexp = parser_parse(&parser);
  cr_assert_not_null(sexp, "s_expression should not be NULL");
  cr_assert_eq(sexp[0]->type, NODE_ATOM,
               "first s_expression should be an atom");
  cr_assert_eq(sexp[0]->data.atom.type, ATOM_NUMBER,
               "atom type should be ATOM_NUMBER");
  cr_assert_eq(sexp[0]->data.atom.value.number, 123, "atom should be 123");
  cr_assert_eq(sexp[1]->type, NODE_ATOM,
               "second s_expression should be an atom");
  cr_assert_eq(sexp[1]->data.atom.type, ATOM_NUMBER,
               "atom type should be ATOM_NUMBER");
  cr_assert_float_eq(sexp[1]->data.atom.value.number, 0.134, 0.001,
                     "atom should be 0.134");
}

Test(parser_tests, it_parses_symbols) {
  const char *input = "foo bar-baz ?qux!";
  lexer_t lexer = lexer_new(input);
  parser_t parser = parser_new(&lexer);
  s_expression_t **sexp = parser_parse(&parser);

  cr_assert_eq(sexp[0]->type, NODE_ATOM,
               "first s_expression should be an atom");
  cr_assert_eq(sexp[0]->data.atom.type, ATOM_SYMBOL,
               "atom type should be ATOM_SYMBOL");
  cr_assert_str_eq(sexp[0]->data.atom.value.symbol, "foo",
                   "first symbol should be 'foo'");

  cr_assert_eq(sexp[1]->data.atom.type, ATOM_SYMBOL,
               "second atom type should be ATOM_SYMBOL");
  cr_assert_str_eq(sexp[1]->data.atom.value.symbol, "bar-baz",
                   "second symbol should be 'bar-baz'");

  cr_assert_eq(sexp[2]->data.atom.type, ATOM_SYMBOL,
               "third atom type should be ATOM_SYMBOL");
  cr_assert_str_eq(sexp[2]->data.atom.value.symbol, "?qux!",
                   "third symbol should be '?qux!'");
}

Test(parser_tests, it_parses_strings) {
  const char *input = "\"hello world\" \"\" \"escaped \\\"quote\\\"\"";
  lexer_t lexer = lexer_new(input);
  parser_t parser = parser_new(&lexer);
  s_expression_t **sexp = parser_parse(&parser);

  cr_assert_eq(sexp[0]->type, NODE_ATOM);
  cr_assert_eq(sexp[0]->data.atom.type, ATOM_STRING);
  cr_assert_str_eq(sexp[0]->data.atom.value.string, "hello world",
                   "should parse a normal string");

  cr_assert_eq(sexp[1]->data.atom.type, ATOM_STRING);
  cr_assert_str_eq(sexp[1]->data.atom.value.string, "",
                   "should parse the empty string");

  cr_assert_eq(sexp[2]->data.atom.type, ATOM_STRING);
  cr_assert_str_eq(sexp[2]->data.atom.value.string, "escaped \"quote\"",
                   "should unescape embedded quotes");
}

Test(parser_tests, it_parses_booleans) {
  const char *input = "#t #f";
  lexer_t lexer = lexer_new(input);
  parser_t parser = parser_new(&lexer);
  s_expression_t **sexp = parser_parse(&parser);

  cr_assert_eq(sexp[0]->type, NODE_ATOM);
  cr_assert_eq(sexp[0]->data.atom.type, ATOM_BOOLEAN);
  cr_assert_eq(sexp[0]->data.atom.value.boolean, true,
               "first boolean should be true");

  cr_assert_eq(sexp[1]->data.atom.type, ATOM_BOOLEAN);
  cr_assert_eq(sexp[1]->data.atom.value.boolean, false,
               "second boolean should be false");
}
