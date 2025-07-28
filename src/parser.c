#include "parser.h"
#include "lexer.h"
#include "string.h"
#include "token.h"
#include <errno.h>
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

s_expression_t *parser_parse_list(parser_t *parser) {}

// typedef enum {
//   ATOM,
//   LIST,
// } node_type_t;
//
// typedef struct s_expression {
//   node_type_t type;
//   union {
//     char *atom;
//     struct {
//       struct s_expression **elements;
//       size_t count;
//     } list;
//   } data;
// } s_expression_t;

s_expression_t *parser_parse_atom(parser_t *parser) {
  atom_t atom = {0};

  switch (parser->current_token.type) {
  case TOKEN_SYMBOL:
    atom.type = ATOM_SYMBOL;
    atom.value.symbol = parser->current_token.literal;
    break;
  case TOKEN_NUMBER:
    atom.type = ATOM_NUMBER;
    char *literal = parser->current_token.literal;
    char *endptr;
    errno = 0;
    double v = strtod(literal, &endptr);

    // 1) No digits were found?
    if (endptr == literal) {
      parser_add_error(parser, "invalid number literal: \"%s\"", literal);
      return NULL;
    }

    // 2) Extra junk after the number?
    if (*endptr != '\0') {
      parser_add_error(parser, "invalid number literal: \"%s\"", literal);
      return NULL;
    }

    // 3) Range errors?
    if (errno == ERANGE) {
      parser_add_error(parser, "number out of range: \"%s\"", literal);
      return NULL;
    }

    atom.value.number = v;
    break;
  case TOKEN_STRING:
    atom.type = ATOM_STRING;
    atom.value.string = parser->current_token.literal;
    break;
  case TOKEN_FALSE:
  case TOKEN_TRUE:
    atom.type = ATOM_BOOLEAN;
    if (strcmp(parser->current_token.literal, "#t") == 0) {
      atom.value.boolean = true;
    } else {
      atom.value.boolean = false;
    }
    break;
  default:
    parser_add_error(parser, "Expected atom but found '%s' at %zu:%zu",
                     parser->current_token.literal, parser->current_token.line,
                     parser->current_token.column);
    return NULL;
  }

  s_expression_t *atom_sexp = malloc(sizeof(s_expression_t));
  if (!atom_sexp) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  atom_sexp->type = NODE_ATOM;
  atom_sexp->data.atom = atom;

  return atom_sexp;
}

s_expression_t *parser_parse_s_expression(parser_t *parser) {
  switch (parser->current_token.type) {
  case TOKEN_SYMBOL:
    return parser_parse_atom(parser);
  case TOKEN_NUMBER:
    return parser_parse_atom(parser);
  case TOKEN_STRING:
    return parser_parse_atom(parser);
  case TOKEN_TRUE:
  case TOKEN_FALSE:
    return parser_parse_atom(parser);
  default:
    fprintf(stderr, "not implemented yet\n");
    return NULL;
  }
}

s_expression_t **parser_parse(parser_t *parser) {
  size_t expr_cap = DEFAULT_EXPRESSION_COUNT;
  s_expression_t **exprs = malloc(expr_cap * sizeof *exprs);
  size_t expr_count = 0;

  while (parser->current_token.type != TOKEN_EOF) {
    s_expression_t *sexp = parser_parse_s_expression(parser);
    if (sexp) {
      exprs[expr_count++] = sexp;
      if (expr_count == expr_cap) {
        expr_cap *= 2;
        exprs = realloc(exprs, expr_cap * sizeof *exprs);
        if (!exprs) {
          perror("realloc");
          exit(EXIT_FAILURE);
        }
      }
    }
    parser_next(parser);
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
