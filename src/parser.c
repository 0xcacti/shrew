#include "parser.h"
#include "lexer.h"
#include "token.h"
#include <stdlib.h>

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

void parser_free(parser_t *parser) {
  if (parser->errors) {
    for (size_t i = 0; i < parser->error_count; i++) {
      free(parser->errors[i]);
    }
    free(parser->errors);
    parser->errors = NULL;
  }
}
