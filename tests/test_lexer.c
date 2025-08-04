#include "lexer.h"
#include <criterion/criterion.h>

Test(lexer_tests, creation) {
  const char *input = "(+ 1 2)";
  lexer_t lexer = lexer_new(input);

  cr_assert_eq(lexer.position, 0, "position should start at 0");
  cr_assert_eq(lexer.read_position, 1, "read_position should start at 1");
  cr_assert_eq(lexer.ch, '(', "first character should be (");
}

Test(lexer_tests, it_lexes_numbers) {
  const char *input = "123";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_NUMBER, "token type should be TOKEN_NUMBER");
  cr_assert_str_eq(token.literal, "123", "token literal should be '123'");
  cr_assert_eq(lexer.position, 3, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_symbols) {
  const char *input = "abc";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_SYMBOL, "token type should be TOKEN_SYMBOL");
  cr_assert_str_eq(token.literal, "abc", "token literal should be 'abc'");
  cr_assert_eq(lexer.position, 3, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_whitespace) {
  char *input = "   ";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");
  cr_assert_eq(lexer.position, 3, "position should be at the end of input");

  input = "  \t\n";

  lexer = lexer_new(input);
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");
  cr_assert_eq(lexer.position, 4, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_parentheses) {
  const char *input = "()";
  lexer_t lexer = lexer_new(input);

  token_t token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_LPAREN, "first token should be TOKEN_LPAREN");
  cr_assert_str_eq(token.literal, "(", "first token literal should be '('");

  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_RPAREN, "second token should be TOKEN_RPAREN");
  cr_assert_str_eq(token.literal, ")", "second token literal should be ')'");

  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_EOF, "third token should be TOKEN_EOF");
}

Test(lexer_tests, it_lexes_strings) {
  const char *input = "\"hello\"";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_STRING, "token type should be TOKEN_STRING");
  cr_assert_str_eq(token.literal, "hello", "token literal should be 'hello'");
  cr_assert_eq(lexer.position, 7, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_single_s_expresions) {
  const char *input = "(+ 1 2)";
  lexer_t lexer = lexer_new(input);
  token_t expected_tokens[6] = {0};
  expected_tokens[0] = token_new(TOKEN_LPAREN, "(");
  expected_tokens[1] = token_new(TOKEN_SYMBOL, "+");
  expected_tokens[2] = token_new(TOKEN_NUMBER, "1");
  expected_tokens[3] = token_new(TOKEN_NUMBER, "2");
  expected_tokens[4] = token_new(TOKEN_RPAREN, ")");
  expected_tokens[5] = token_new(TOKEN_EOF, "");

  for (int i = 0; i < 6; i++) {
    token_t token = lexer_next_token(&lexer);
    cr_assert_eq(token.type, expected_tokens[i].type, "token type mismatch");
    cr_assert_str_eq(token.literal, expected_tokens[i].literal,
                     "token literal mismatch");
  }
}

Test(lexer_tests, it_lexes_multiple_s_expressions) {
  const char *input = "(+ 1 2) (- 3 4)";
  lexer_t lexer = lexer_new(input);
  token_t expected_tokens[12] = {0};
  expected_tokens[0] = token_new(TOKEN_LPAREN, "(");
  expected_tokens[1] = token_new(TOKEN_SYMBOL, "+");
  expected_tokens[2] = token_new(TOKEN_NUMBER, "1");
  expected_tokens[3] = token_new(TOKEN_NUMBER, "2");
  expected_tokens[4] = token_new(TOKEN_RPAREN, ")");
  expected_tokens[5] = token_new(TOKEN_LPAREN, "(");
  expected_tokens[6] = token_new(TOKEN_SYMBOL, "-");
  expected_tokens[7] = token_new(TOKEN_NUMBER, "3");
  expected_tokens[8] = token_new(TOKEN_NUMBER, "4");
  expected_tokens[9] = token_new(TOKEN_RPAREN, ")");
  expected_tokens[10] = token_new(TOKEN_EOF, "");

  for (int i = 0; i < 11; i++) {
    token_t token = lexer_next_token(&lexer);
    cr_assert_eq(token.type, expected_tokens[i].type,
                 "token type mismatch at index %d", i);
    cr_assert_str_eq(token.literal, expected_tokens[i].literal,
                     "token literal mismatch at index %d", i);
  }
}

Test(lexer_tests, it_lexes_nested_s_expressions) {
  const char *input = "(+ 1 (- 2 3))";
  lexer_t lexer = lexer_new(input);
  token_t expected_tokens[10] = {0};
  expected_tokens[0] = token_new(TOKEN_LPAREN, "(");
  expected_tokens[1] = token_new(TOKEN_SYMBOL, "+");
  expected_tokens[2] = token_new(TOKEN_NUMBER, "1");
  expected_tokens[3] = token_new(TOKEN_LPAREN, "(");
  expected_tokens[4] = token_new(TOKEN_SYMBOL, "-");
  expected_tokens[5] = token_new(TOKEN_NUMBER, "2");
  expected_tokens[6] = token_new(TOKEN_NUMBER, "3");
  expected_tokens[7] = token_new(TOKEN_RPAREN, ")");
  expected_tokens[8] = token_new(TOKEN_RPAREN, ")");
  expected_tokens[9] = token_new(TOKEN_EOF, "");

  for (int i = 0; i < 10; i++) {
    token_t token = lexer_next_token(&lexer);
    cr_assert_eq(token.type, expected_tokens[i].type,
                 "token type mismatch at index %d", i);
    cr_assert_str_eq(token.literal, expected_tokens[i].literal,
                     "token literal mismatch at index %d", i);
  }
}

Test(lexer_test, it_lexes_negative_numbers) {
  const char *input = "-123";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_NUMBER, "token type should be TOKEN_NUMBER");
  cr_assert_str_eq(token.literal, "-123", "token literal should be '-123'");
  cr_assert_eq(lexer.position, 4, "position should be at the end of input");
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");
  cr_assert_eq(lexer.position, 4,
               "position should still be at the end of input");
}

Test(lexer_test, it_lexes_float_numbers) {
  const char *input = "3.14";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_NUMBER, "token type should be TOKEN_NUMBER");
  cr_assert_str_eq(token.literal, "3.14", "token literal should be '3.14'");
  cr_assert_eq(lexer.position, 4, "position should be at the end of input");

  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");

  input = ".99";
  lexer = lexer_new(input);
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_NUMBER, "token type should be TOKEN_NUMBER");
  cr_assert_str_eq(token.literal, ".99", "token literal should be '.99'");
  cr_assert_eq(lexer.position, 3, "position should be at the end of input");
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");

  input = "-0.123";
  lexer = lexer_new(input);
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_NUMBER, "token type should be TOKEN_NUMBER");
  cr_assert_str_eq(token.literal, "-0.123", "token literal should be '-0.123'");
  cr_assert_eq(lexer.position, 6, "position should be at the end of input");
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");

  input = "-.456";
  lexer = lexer_new(input);
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_NUMBER, "token type should be TOKEN_NUMBER");
  cr_assert_str_eq(token.literal, "-.456", "token literal should be '-.456'");
  cr_assert_eq(lexer.position, 5, "position should be at the end of input");
  token = lexer_next_token(&lexer);
  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");
}

Test(lexer_tests, it_lexes_quote) {
  const char *input = "'foo";
  lexer_t lexer = lexer_new(input);

  token_t t;

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_QUOTE, "first token should be TOKEN_QUOTE");
  cr_assert_str_eq(t.literal, "'", "first literal should be \"'\"");

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_SYMBOL, "second token should be symbol");
  cr_assert_str_eq(t.literal, "foo", "second literal should be \"foo\"");
}

Test(lexer_tests, it_lexes_quasiquote) {
  const char *input = "`(a b)";
  lexer_t lexer = lexer_new(input);

  token_t t;

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_QUASIQUOTE,
               "first token should be TOKEN_QUASIQUOTE");
  cr_assert_str_eq(t.literal, "`", "literal should be \"`\"");

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_LPAREN, "next token should be LPAREN");
}

Test(lexer_tests, it_lexes_unquote) {
  const char *input = ",x";
  lexer_t lexer = lexer_new(input);

  token_t t;

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_UNQUOTE, "first token should be TOKEN_UNQUOTE");
  cr_assert_str_eq(t.literal, ",", "literal should be \",\"");

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_SYMBOL, "next token should be symbol");
  cr_assert_str_eq(t.literal, "x", "literal should be \"x\"");
}

Test(lexer_tests, it_lexes_unquote_splicing) {
  const char *input = ",@rest";
  lexer_t lexer = lexer_new(input);

  token_t t;

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_UNQUOTE_SPLICING,
               "first token should be TOKEN_UNQUOTE_SPLICING");
  cr_assert_str_eq(t.literal, ",@", "literal should be \",@\"");

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_SYMBOL, "next token should be symbol");
  cr_assert_str_eq(t.literal, "rest", "literal should be \"rest\"");
}

Test(lexer_tests, it_lexes_true_and_false) {
  const char *input = "#t #f";
  lexer_t lexer = lexer_new(input);

  token_t t;

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_TRUE, "first token should be TOKEN_TRUE");
  cr_assert_str_eq(t.literal, "#t", "literal should be \"#t\"");

  t = lexer_next_token(&lexer);
  cr_assert_eq(t.type, TOKEN_FALSE, "next token should be TOKEN_FALSE");
  cr_assert_str_eq(t.literal, "#f", "literal should be \"#f\"");
}

Test(lexer_tests, it_lexes_invalid_tokens) {
  const char *input = "#~";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_INVALID, "token type should be TOKEN_INVALID");
  cr_assert_str_eq(token.literal, "#~", "token literal should be '#~'");
  cr_assert_eq(lexer.position, 2, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_empty_input) {
  const char *input = "";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");
  cr_assert_eq(lexer.position, 0, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_unterminated_string) {
  const char *input = "\"unterminated";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_INVALID, "token type should be TOKEN_INVALID");
  cr_assert_str_eq(token.literal, "unterminated",
                   "token literal should be '\"unterminated'");
  cr_assert_eq(lexer.position, 13, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_invalid_number) {
  char *input = "1.2.3";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_INVALID, "token type should be TOKEN_INVALID");
  cr_assert_str_eq(token.literal, "1.2.3", "token literal should be '1.2.3'");
  cr_assert_eq(lexer.position, 5, "position should be at the end of input");
}

Test(lexer_tests, it_lexes_invalid_at_sign) {
  const char *input = "@invalid";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_INVALID, "token type should be TOKEN_INVALID");
  cr_assert_str_eq(token.literal, "@", "token literal should be '@'");
  cr_assert_eq(lexer.position, 1, "position should be at the end of input");
}

Test(lexer_tests, reports_line_and_column_on_invalid_number) {
  const char *input = "\n"
                      "1.2.3";

  lexer_t lx = lexer_new(input);
  token_t t = lexer_next_token(&lx);

  cr_assert_eq(t.type, TOKEN_INVALID);
  cr_assert_str_eq(t.literal, "1.2.3");

  cr_assert_eq(t.line, 2, "line should be 2");
  cr_assert_eq(t.column, 1, "column should be 1");
}

Test(lexer_tests, reports_position_of_stray_dot) {
  const char *input = "(+ 1 3) . (- 3 1)";
  lexer_t lx = lexer_new(input);

  token_t tok;
  do {
    tok = lexer_next_token(&lx);
  } while (tok.type != TOKEN_DOT);

  cr_assert_eq(tok.type, TOKEN_DOT);
  cr_assert_eq(tok.line, 1);
  cr_assert_eq(tok.column, 9);
}

Test(lexer_tests, skips_many_comments_and_space) {
  const char *input = "   ; first comment \n"
                      " ; second comment\n"
                      "\t\tfoo";
  lexer_t lx = lexer_new(input);
  token_t t = lexer_next_token(&lx);

  cr_assert_eq(t.type, TOKEN_SYMBOL);
  cr_assert_str_eq(t.literal, "foo");
  cr_assert_eq(t.line, 3);
  cr_assert_eq(t.column, 3);
}

Test(number_tests, accepts_various_valid_forms) {
  const char *src = "123 -42 1. .5 1.0 +.7";
  lexer_t lx = lexer_new(src);
  const char *expect[] = {"123", "-42", "1.", ".5", "1.0", "+.7"};

  for (size_t i = 0; i < sizeof expect / sizeof *expect; ++i) {
    token_t t = lexer_next_token(&lx);
    cr_assert_eq(t.type, TOKEN_NUMBER);
    cr_assert_str_eq(t.literal, expect[i]);
  }
}

Test(number_tests, rejects_invalid_numbers) {
  const char *src = "-. .. 1.2.3";
  lexer_t lx = lexer_new(src);

  token_t t = lexer_next_token(&lx);
  cr_assert_eq(t.type, TOKEN_INVALID);

  t = lexer_next_token(&lx);
  cr_assert_eq(t.type, TOKEN_DOT);
  t = lexer_next_token(&lx);
  cr_assert_eq(t.type, TOKEN_DOT);

  t = lexer_next_token(&lx);
  cr_assert_eq(t.type, TOKEN_INVALID);
}
