#include "lexer.h"
#include "string.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

void read_char(lexer_t *lexer) {
  if (lexer->read_position >= lexer->input_len) {
    lexer->ch = 0;
  } else {
    lexer->ch = lexer->input[lexer->read_position];
  }

  lexer->position = lexer->read_position;
  lexer->read_position += 1;
}

void skip_whitespace(lexer_t *lexer) {
  while (lexer->ch == ' ' || lexer->ch == '\t' || lexer->ch == '\n' ||
         lexer->ch == '\r') {
    read_char(lexer);
  }
}

void skip_line_comment(lexer_t *lexer) {
  while (lexer->ch != '\n' && lexer->ch != 0) {
    read_char(lexer);
  }
  if (lexer->ch != 0) {
    read_char(lexer);
  }
}

char *read_string(lexer_t *lexer) {
  size_t start_pos = lexer->position + 1;
  while (1) {
    read_char(lexer);
    if (lexer->ch == '"' || lexer->ch == 0) {
      break;
    }
  }

  return strndup(lexer->input + start_pos, lexer->position - start_pos);
}

char *read_number(lexer_t *lexer) {
  size_t start_pos = lexer->position;
  while (isdigit(lexer->ch) || lexer->ch == '.') {
    read_char(lexer);
  }
  return strndup(lexer->input + start_pos, lexer->position - start_pos);
}

char *read_symbol(lexer_t *lexer) {
  size_t start_pos = lexer->position;
  while (lexer->ch != ' ' && lexer->ch != '\t' && lexer->ch != '\n' &&
         lexer->ch != '\r' && lexer->ch != '(' && lexer->ch != ')' &&
         lexer->ch != '"' && lexer->ch != '\'' && lexer->ch != '@' &&
         lexer->ch != 0) {
    read_char(lexer);
  }
  size_t length = lexer->position - start_pos;
  if (length == 0) {
    return NULL;
  }
  return strndup(lexer->input + start_pos, length);
}

char peek(lexer_t *lexer) {
  if (lexer->read_position >= lexer->input_len) {
    return 0;
  } else {
    return lexer->input[lexer->read_position];
  }
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

token_t lexer_next_token(lexer_t *lexer) {
  skip_whitespace(lexer);
  if (lexer->ch == ';') {
    skip_line_comment(lexer);
  }

  token_t token = {0};
  switch (lexer->ch) {
    case 0: 
      token = token_new(TOKEN_EOF, "");
      break;
    case '(': 
      token = token_new(TOKEN_LPAREN, "(");
      break;
    case ')':
      token = token_new(TOKEN_RPAREN, ")");
      break;
    case '"': 
      token = token_new(TOKEN_STRING, read_string(lexer));
      break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      token = token_new(TOKEN_NUMBER, read_number(lexer));
      break;
    default: 
      token = token_new(TOKEN_SYMBOL, read_symbol(lexer));
      break;
  }

  read_char(lexer);
  return token;
}

