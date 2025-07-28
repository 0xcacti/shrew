#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"

typedef enum {
  ATOM,
  LIST,
} node_type_t;

typedef struct s_expression {
  node_type_t type;
  union {
    char *atom;
    struct {
      struct s_expression **elements;
      size_t count;
    } list;
  } data;
} s_expression_t; 

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
s_expression_t **parser_parse(parser_t *parser);

#endif
