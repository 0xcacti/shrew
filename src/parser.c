#include "parser.h"
#include "lexer.h"
#include "token.h"
#include <stdarg.h>
#include <stdlib.h>

const int DEFAULT_EXPRESSION_COUNT = 16;

static void grow_error_capacity(parser_t *parser) {
  size_t new_cap = parser->error_capacity ? parser->error_capacity * 2 : 4;
  char **new = realloc(parser->errors, new_cap * sizeof(char *));
  if (!new) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }
  parser->errors = new;
  parser->error_capacity = new_cap;
}

void parser_add_error(parser_t *parser, const char *fmt, ...) {
  if (parser->error_count + 1 > parser->error_capacity) {
    grow_error_capacity(parser);
  }

  va_list ap;
  va_start(ap, fmt);
  char tmp[256];
  int needed = vsnprintf(tmp, sizeof tmp, fmt, ap);
  va_end(ap);
  if (needed < 0) {
    perror("vsnprintf");
    exit(EXIT_FAILURE);
  }

  char *msg = malloc((size_t)needed + 1);
  if (!msg) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  va_start(ap, fmt);
  vsnprintf(msg, (size_t)needed + 1, fmt, ap);
  va_end(ap);

  parser->errors[parser->error_count++] = msg;
}

void parser_clear_errors(parser_t *parser) {
  for (size_t i = 0; i < parser->error_count; i++) {
    free(parser->errors[i]);
  }
  free(parser->errors);
  parser->errors = NULL;
  parser->error_count = parser->error_capacity = 0;
}

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

void parser_next(parser_t *parser) {
  parser->current_token = parser->next_token;
  parser->next_token = lexer_next_token(parser->lexer);
}

int parser_parse_s_expression(parser_t *parser, s_expression_t **out) {

  return 0;
}

s_expression_t **parser_parse(parser_t *parser) {
  size_t expr_cap = DEFAULT_EXPRESSION_COUNT;
  s_expression_t **exprs = malloc(expr_cap * sizeof *exprs);
  size_t expr_count = 0;

  while (parser->current_token.type != TOKEN_EOF) {
    s_expression_t *sexp = NULL;
    int status = parser_parse_s_expression(parser, &sexp);
    if (status == 0) {
      exprs[expr_count++] = sexp;
      if (expr_count == expr_cap) {
        expr_cap *= 2;
        exprs = realloc(exprs, expr_cap * sizeof *exprs);
        if (!exprs) {
          perror("realloc");
          exit(EXIT_FAILURE);
        }
      }
    } else {
      parser_add_error(parser, "Parse error at %zu:%zu, skipping token '%s'",
                       parser->current_token.line, parser->current_token.column,
                       parser->current_token.literal);
      parser_next(parser);
    }
  }
  return exprs;
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
