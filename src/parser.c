#include "parser.h"
#include "lexer.h"
#include "token.h"
#include <stdlib.h>

const int DEFAULT_EXPRESSION_COUNT = 16;

parser_t parser_new(lexer_t *lexer) {
  // clang-format off
  parser_t parser = {
    .lexer = lexer,
    .current_token = lexer_next_token(lexer),
    .next_token = lexer_next_token(lexer),
    .errors = NULL,
    .error_count = 0,
    .error_capacity = 0
  };
  // clang-format on

  parser.error_capacity = 4;
  parser.errors = malloc(parser.error_capacity * sizeof(char *));

  return parser;
}

s_expression_t *parser_parse_s_expression(parser_t *parser) {
  // placeholder
  return NULL;
}

s_expression_t **parser_parse(parser_t *parser) {
  // our goal is to return a list of s expressions
  // we will use a recursive descent parser
  // while we are not at the end of the input
  // we will parse s expression
  s_expression_t **expressions =
      malloc(DEFAULT_EXPRESSION_COUNT * sizeof(s_expression_t *));
  size_t expression_count = 0;
  size_t expression_capacity = DEFAULT_EXPRESSION_COUNT;

  size_t error_count = 0;
  while (parser->current_token.type != TOKEN_EOF) {
    // dispatch to parse_expression, figure out error handling
    s_expression_t *s_exp = parser_parse_s_expression(parser);
    if (parser->error_count > error_count) {
    }
  }

  return NULL;
}

void parser_free(parser_t *parser) {
  if (parser->errors) {
    for (size_t i = 0; i < parser->error_count; i++) {
      free(parser->errors[i]);
    }
    free(parser->errors);
    parser->errors = NULL;
  }
}
