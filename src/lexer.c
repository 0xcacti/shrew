#include "lexer.h"
#include "string.h"

void read_char(lexer_t *lexer) {
  if (lexer->read_position >= lexer->input_len) {
    lexer->ch = 0;
  } else {
    lexer->ch = lexer->input[lexer->read_position];
  }

  lexer->position = lexer->read_position;
  lexer->read_position += 1;
}

lexer_t lexer_new(const char *input) {
  size_t input_len = strlen(input);
  // clang-format off
  lexer_t lexer = {
    .position = 0,
    .read_position = 0,
    .ch = 0,
    .input = input,
    .input_len = input_len
  };

  read_char(&lexer);
  return lexer;
}

token_t lexer_next_token(lexer_t *lexer) {}

