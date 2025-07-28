#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"

typedef struct parser {
  lexer_t *lexer;
  token_t current_token;
  token_t next_token;
  char **errors;
  size_t error_count;
  size_t error_capacity;
} parser_t;

parser_t parser_new(lexer_t *lexer);
void parser_free(parser_t *parser);

#endif
