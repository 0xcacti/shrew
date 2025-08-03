#include "parser.h"
#include "lexer.h"
#include "string.h"
#include "token.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

const int DEFAULT_EXPRESSION_COUNT = 16;
static s_expression_t *parser_parse_s_expression(parser_t *parser);

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

s_expression_t *parser_parse_list(parser_t *parser) {
  if (parser->current_token.type != TOKEN_LPAREN) {
    parser_add_error(
        parser, "internal: parser_parse_list called when current token is '%s'",
        parser->current_token.literal);
    return NULL;
  }
  parser_next(parser);

  size_t capacity = DEFAULT_EXPRESSION_COUNT;
  size_t count = 0;
  s_expression_t **elements = malloc(capacity * sizeof(*elements));
  if (!elements) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  bool saw_dot = false;
  s_expression_t *dotted_tail = NULL;
  while (parser->current_token.type != TOKEN_RPAREN) {
    if (parser->current_token.type == TOKEN_EOF) {
      parser_add_error(parser, "unexpected end-of-file while parsing list");
      free(elements);
      return NULL;
    }
    if (parser->current_token.type == TOKEN_DOT) {
      if (saw_dot) {
        parser_add_error(parser, "multiple dots in list");
        free(elements);
        return NULL;
      }
      s_expression_t *last_elem = elements[count - 1];
      if (!last_elem) {
        parser_add_error(parser, "leading dot in list");
        free(elements);
        return NULL;
      }
      saw_dot = true;
      parser_next(parser);
      dotted_tail = parser_parse_s_expression(parser);
      if (!dotted_tail) {
        free(elements);
        return NULL;
      }
      parser_next(parser);

      if (parser->current_token.type != TOKEN_RPAREN) {
        if (parser->current_token.type == TOKEN_DOT) {
          parser_add_error(parser, "multiple dots in list");
        } else {
          parser_add_error(parser, "expected token after dotted tail");
        }
        free(elements);

        while (parser->current_token.type != TOKEN_RPAREN &&
               parser->current_token.type != TOKEN_EOF) {
          parser_next(parser);
        }
        return NULL;
      }
      break;
    }

    s_expression_t *element = parser_parse_s_expression(parser);
    if (!element) {
      free(elements);
      return NULL;
    }

    if (count == capacity) {
      capacity *= 2;
      elements = realloc(elements, capacity * sizeof(s_expression_t *));
      if (!elements) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
    }
    elements[count++] = element;
    parser_next(parser);
  }

  if (parser->current_token.type != TOKEN_RPAREN) {
    parser_add_error(parser, "expected ')' but found '%s'",
                     parser->current_token.literal);
    free(elements);
    return NULL;
  }

  s_expression_t *list_sexp = malloc(sizeof(s_expression_t));
  if (!list_sexp) {
    perror("malloc");
    free(elements);
    exit(EXIT_FAILURE);
  }
  list_sexp->type = NODE_LIST;
  list_sexp->data.list.elements = elements;
  list_sexp->data.list.count = count;
  if (dotted_tail) {
    list_sexp->data.list.tail = dotted_tail;
  }
  return list_sexp;
}

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

    if (endptr == literal) {
      parser_add_error(parser, "invalid number literal: \"%s\"", literal);
      return NULL;
    }

    if (*endptr != '\0') {
      parser_add_error(parser, "invalid number literal: \"%s\"", literal);
      return NULL;
    }

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

s_expression_t *parser_parse_quote_family(parser_t *parser) {
  switch (parser->current_token.type) {
  case TOKEN_QUOTE:
    parser_next(parser); // consume '
    s_expression_t *quoted = parser_parse_s_expression(parser);
    if (!quoted) { // SHOULD I HAVE ERROR HANDLING HERE
      return NULL;
    }

    atom_t quote_atom = {0};
    quote_atom.type = ATOM_SYMBOL;
    quote_atom.value.symbol = "quote";

    s_expression_t *quote_symbol = malloc(sizeof(s_expression_t));
    if (!quote_symbol) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    quote_symbol->type = NODE_ATOM;
    quote_symbol->data.atom = quote_atom;

    switch (quoted->type) {
    case NODE_ATOM: {
      s_expression_t *prev_atom = malloc(sizeof(s_expression_t));
      if (!prev_atom) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }

      // first obvious place where errors could occur
      memcpy(prev_atom, quoted, sizeof(s_expression_t));

      quoted->type = NODE_LIST;
      s_expression_t **elements = malloc(2 * sizeof(*elements));
      if (!elements) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      elements[0] = quote_symbol;
      elements[1] = prev_atom;

      quoted->data.list.count = 2;
      quoted->data.list.elements = elements;
      return quoted;
    }
    case NODE_LIST:
      s_expression_t *prev_list = malloc(sizeof(s_expression_t));
      if (!prev_list) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      memcpy(prev_list, quoted, sizeof(s_expression_t));
    }

  default:
    parser_add_error(parser, "tried to parse quote family erroneously");
    return NULL;
  }
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
  case TOKEN_LPAREN:
    return parser_parse_list(parser);
  case TOKEN_RPAREN:
    parser_add_error(parser, "unexpected ')' at %zu:%zu",
                     parser->current_token.line, parser->current_token.column);
    return NULL;
  case TOKEN_EOF:
    parser_add_error(parser, "unexpected end-of-file at %zu:%zu",
                     parser->current_token.line, parser->current_token.column);
    return NULL;
  case TOKEN_INVALID:
    parser_add_error(parser, "invalid token '%s' at %zu:%zu",
                     parser->current_token.literal, parser->current_token.line,
                     parser->current_token.column);
    return NULL;
  case TOKEN_DOT:
    parser_add_error(parser, "saw dot outside of list at %zu:%zu",
                     parser->current_token.literal, parser->current_token.line,
                     parser->current_token.column);
    return NULL;
  case TOKEN_QUOTE:
    return parser_parse_quote_family(parser);
  default:
    fprintf(stderr, "not implemented yet\n");
    exit(EXIT_FAILURE);
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
