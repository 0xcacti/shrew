#include "lexer.h"
#include "string.h"
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

  size_t length = lexer->position - start_pos;
  if (length == 0) {
    return NULL;
  }

  char *str = malloc(length + 1);
  if (!str) {
    perror("malloc");
    return NULL; // Memory allocation failed
  }
  strncpy(str, lexer->input + start_pos, length);
  str[length] = '\0';
  return str;
}

int read_int(lexer_t *lexer) {
  size_t start_pos = lexer->position;
  while (1) {
    read_char(lexer);
    if (!(lexer->ch >= '0' || lexer->ch <= '9') || lexer->ch == 0) {
      break;
    }
  }

  size_t length = lexer->position - start_pos;
  char *str = malloc(length + 1);
  if (!str) {
    perror("malloc");
    return 0;
  }

  strncpy(str, lexer->input + start_pos, length);
  str[length] = '\0';
  int val = atoi(str);
  free(str);
  return val;
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

 //  TOKEN_EOF,
 //  TOKEN_LPAREN,
 //  TOKEN_RPAREN,
 //  TOKEN_STRING
 //  TOKEN_NUMBER, 
 //  TOKEN_SYMBOL, 
 //  TOKEN_INVALID, 

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
  }

  read_char(lexer);
  return token;
}

