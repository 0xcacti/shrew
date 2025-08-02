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

Test(parser_tests, it_parses_lists) {
  const char *input = "(1 2 3) (foo bar (baz qux))";
  lexer_t lexer = lexer_new(input);
  parser_t parser = parser_new(&lexer);
  s_expression_t **sexp = parser_parse(&parser);
  cr_assert_eq(parser.error_count, 0, "there should be no parsing errors");
  cr_assert_eq(sexp[0]->type, NODE_LIST, "first s_expression should be a list");
  cr_assert_eq(sexp[0]->data.list.count, 3,
               "first list should have 3 elements");
  cr_assert_eq(sexp[0]->data.list.elements[0]->type, NODE_ATOM,
               "first element of first list should be an atom");
  cr_assert_eq(sexp[0]->data.list.elements[0]->data.atom.type, ATOM_NUMBER,
               "first element of first list should be a number atom");
  cr_assert_float_eq(sexp[0]->data.list.elements[0]->data.atom.value.number, 1,
                     0.001, "first element of first list should be 1");
  cr_assert_eq(sexp[0]->data.list.elements[1]->type, NODE_ATOM,
               "second element of first list should be an atom");
  cr_assert_eq(sexp[0]->data.list.elements[1]->data.atom.type, ATOM_NUMBER,
               "second element of first list should be a number atom");
  cr_assert_float_eq(sexp[0]->data.list.elements[1]->data.atom.value.number, 2,
                     0.001, "second element of first list should be 2");
  cr_assert_eq(sexp[0]->data.list.elements[2]->type, NODE_ATOM,
               "third element of first list should be an atom");
  cr_assert_eq(sexp[0]->data.list.elements[2]->data.atom.type, ATOM_NUMBER,
               "third element of first list should be a number atom");
  cr_assert_float_eq(sexp[0]->data.list.elements[2]->data.atom.value.number, 3,
                     0.001, "third element of first list should be 3");
  printf("we get to here\n");
  printf("sexp[1]->type: %d\n", sexp[1]->type);

  cr_assert_eq(sexp[1]->type, NODE_LIST,
               "second s_expression should be a list");

  cr_assert_eq(sexp[1]->data.list.count, 3,
               "second list should have 3 elements");
  cr_assert_eq(sexp[1]->data.list.elements[0]->type, NODE_ATOM,
               "first element of second list should be an atom");
  cr_assert_eq(sexp[1]->data.list.elements[0]->data.atom.type, ATOM_SYMBOL,
               "first element of second list should be a symbol atom");
  cr_assert_str_eq(sexp[1]->data.list.elements[0]->data.atom.value.symbol,
                   "foo", "first element of second list should be 'foo'");
  cr_assert_eq(sexp[1]->data.list.elements[1]->type, NODE_ATOM,
               "second element of second list should be an atom");
  cr_assert_eq(sexp[1]->data.list.elements[1]->data.atom.type, ATOM_SYMBOL,
               "second element of second list should be a symbol atom");
  cr_assert_str_eq(sexp[1]->data.list.elements[1]->data.atom.value.symbol,
                   "bar", "second element of second list should be 'bar'");
  cr_assert_eq(sexp[1]->data.list.elements[2]->type, NODE_LIST,
               "third element of second list should be a list");
  cr_assert_eq(sexp[1]->data.list.elements[2]->data.list.count, 2,
               "third element of second list should have 2 elements");
  cr_assert_eq(sexp[1]->data.list.elements[2]->data.list.elements[0]->type,
               NODE_ATOM, "first element of third list should be an atom");
  cr_assert_eq(
      sexp[1]->data.list.elements[2]->data.list.elements[0]->data.atom.type,
      ATOM_SYMBOL, "first element of third list should be a symbol atom");
  cr_assert_str_eq(sexp[1]
                       ->data.list.elements[2]
                       ->data.list.elements[0]
                       ->data.atom.value.symbol,
                   "baz", "first element of third list should be 'baz'");
  cr_assert_eq(sexp[1]->data.list.elements[2]->data.list.elements[1]->type,
               NODE_ATOM, "second element of third list should be an atom");
  cr_assert_eq(
      sexp[1]->data.list.elements[2]->data.list.elements[1]->data.atom.type,
      ATOM_SYMBOL, "second element of third list should be a symbol atom");
  cr_assert_str_eq(sexp[1]
                       ->data.list.elements[2]
                       ->data.list.elements[1]
                       ->data.atom.value.symbol,
                   "qux", "second element of third list should be 'qux'");
  for (size_t i = 0; i < parser.error_count; i++) {
    fprintf(stderr, "Error: %s\n", parser.errors[i]);
  }
}
