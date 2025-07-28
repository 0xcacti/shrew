#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

typedef enum token_type {
  TOKEN_INVALID, 
  TOKEN_EOF,
  TOKEN_DOT,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_SYMBOL, 
  TOKEN_NUMBER, 
  TOKEN_STRING,
  TOKEN_QUOTE,
  TOKEN_UNQUOTE,
  TOKEN_QUASIQUOTE,
  TOKEN_UNQUOTE_SPLICING,
  TOKEN_TRUE, 
  TOKEN_FALSE, 
} token_type_t;


typedef struct token {
  token_type_t type;
  char *literal;
  size_t line;
  size_t column;
} token_t;

token_t token_new(token_type_t type, const char *literal);
void token_free(token_t *token);

#endif
