#ifndef TOKEN_H
#define TOKEN_H

typedef enum token_type {
  TOKEN_INVALID, 
  TOKEN_EOF,
  TOKEN_DOT,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_SYMBOL, 
  TOKEN_NUMBER, 
  TOKEN_STRING
} token_type_t;


typedef struct token {
  token_type_t type;
  char *literal;
} token_t;

token_t token_new(token_type_t type, const char *literal);
void token_free(token_t *token);

#endif
