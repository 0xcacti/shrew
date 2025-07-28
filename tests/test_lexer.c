#include "lexer.h"
#include <criterion/criterion.h>

Test(lexer_tests, creation) {
  const char *input = "(+ 1 2)";
  lexer_t lexer = lexer_new(input);

  cr_assert_eq(lexer.position, 0, "position should start at 0");
  cr_assert_eq(lexer.read_position, 1, "read_position should start at 1");
  cr_assert_eq(lexer.ch, '(', "first character should be (");
}

Test(lexer_tests, simple_number) {
  const char *input = "123";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_NUMBER, "token type should be TOKEN_NUMBER");
  cr_assert_str_eq(token.literal, "123", "token literal should be '123'");
  cr_assert_eq(lexer.position, 3, "position should be at the end of input");
}

Test(lexer_tests, simple_symbol) {
  const char *input = "abc";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_SYMBOL, "token type should be TOKEN_SYMBOL");
  cr_assert_str_eq(token.literal, "abc", "token literal should be 'abc'");
  cr_assert_eq(lexer.position, 3, "position should be at the end of input");
}

Test(lexer_tests, simple_whitespace) {
  const char *input = "   ";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  cr_assert_eq(token.type, TOKEN_EOF, "token type should be TOKEN_EOF");
  cr_assert_str_eq(token.literal, "", "token literal should be empty");
  cr_assert_eq(lexer.position, 3, "position should be at the end of input");
}

Test(lexer_tests, simple_parentheses) {
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

Test(lexer_tests, simple_string) {
  const char *input = "\"hello\"";
  lexer_t lexer = lexer_new(input);
  token_t token = lexer_next_token(&lexer);

  printf("Token type: %d, literal: %s\n", token.type, token.literal);
  cr_assert_eq(token.type, TOKEN_STRING, "token type should be TOKEN_STRING");
  cr_assert_str_eq(token.literal, "hello", "token literal should be 'hello'");
  cr_assert_eq(lexer.position, 7, "position should be at the end of input");
}

// Test(lexer_tests, simple_s_expression) {
//   const char *input = "(+ 1 2)";
//   lexer_t lexer = lexer_new(input);
//   token_t expected_tokens[6] = {0};
//   expected_tokens[0] = token_new(TOKEN_LPAREN, "(");
//   expected_tokens[1] = token_new(TOKEN_SYMBOL, "+");
//   expected_tokens[2] = token_new(TOKEN_NUMBER, "1");
//   expected_tokens[3] = token_new(TOKEN_NUMBER, "2");
//   expected_tokens[4] = token_new(TOKEN_RPAREN, ")");
//   expected_tokens[5] = token_new(TOKEN_EOF, "");
// }
