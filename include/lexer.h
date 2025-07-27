#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "token.h"

typedef struct lexer {
  size_t position;
  size_t read_position;
  char ch;
  char *input;
} lexer_t;

lexer_t lexer_new(const char *input);
token_t lexer_next_token(lexer_t *lexer);

#endif
