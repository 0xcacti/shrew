#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "token.h"
#include "lexer.h"

typedef enum {
  ATOM_SYMBOL,
  ATOM_NUMBER,
  ATOM_STRING,
  ATOM_BOOLEAN, 
} atom_type_t;

typedef struct atom {
  atom_type_t type;
  union {
    char *symbol;   // For ATOM_SYMBOL
    double number;  // For ATOM_NUMBER
    char *string;   // For ATOM_STRING
    bool boolean;    // For ATOM_BOOLEAN (0 or 1)
  } value;
} atom_t;

typedef enum {
  NODE_ATOM,
  NODE_LIST,
} node_type_t;

typedef struct s_expression {
  node_type_t type;
  union {
    atom_t atom;
    struct {
      struct s_expression **elements;
      size_t count;
      struct s_expression *tail;
    } list;
  } data;
} s_expression_t; 

typedef struct parse_result {
  s_expression_t **expressions;
  size_t count;
} parse_result_t;

typedef struct parser {
  lexer_t *lexer;
  token_t current_token;
  token_t next_token;

  char **errors;
  size_t error_count;
  size_t error_capacity;

  int qq_depth;
} parser_t;

bool sexp_is_symbol(const s_expression_t *sexp, const char **out_name);
bool sexp_is_symbol_name(const s_expression_t *e, const char *name);
void parser_add_error(parser_t *parser, const char *fmt, ...);
void parser_clear_errors(parser_t *parser);
parser_t parser_new(lexer_t *lexer);
void parser_free(parser_t *parser);
void parse_result_free(parse_result_t *result);
parse_result_t parser_parse(parser_t *parser);

#endif
