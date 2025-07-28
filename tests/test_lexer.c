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

  printf("Token type: %d, literal: %s\n", token.type, token.literal);
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
