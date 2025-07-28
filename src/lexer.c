#include "lexer.h"
#include "string.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

void read_char(lexer_t *lexer) {
  if (lexer->read_position >= lexer->input_len) {
    lexer->ch = 0;
  } else {
    lexer->ch = lexer->input[lexer->read_position];
  }

  if (lexer->ch == '\n') {
    lexer->line++;
    lexer->column = 0;
  } else {
    lexer->column++;
  }

  lexer->position = lexer->read_position;
  lexer->read_position += 1;
}

void skip_whitespace(lexer_t *lexer) {
  while (lexer->ch == ' ' || lexer->ch == '\t' || lexer->ch == '\n' ||
         lexer->ch == '\r') {
    read_char(lexer);
  }
}

void skip_line_comment(lexer_t *lexer) {
  while (lexer->ch != '\n' && lexer->ch != 0) {
    read_char(lexer);
  }
  if (lexer->ch != 0) {
    read_char(lexer);
  }
}

char *read_string(lexer_t *lexer, bool *ok) {
  read_char(lexer);

  size_t start = lexer->position;

  while (lexer->ch != '"' && lexer->ch != 0) {
    read_char(lexer);
  }

  if (lexer->ch == '"') {
    *ok = true;
  } else {
    *ok = false;
  }

  size_t end = lexer->position;
  char *lit = strndup(lexer->input + start, end - start);

  if (*ok) {
    read_char(lexer);
  }

  return lit;
}

char peek(lexer_t *lexer) {
  if (lexer->read_position >= lexer->input_len) {
    return 0;
  } else {
    return lexer->input[lexer->read_position];
  }
}

char *read_number(lexer_t *lexer, bool *ok) {
  *ok = true;
  size_t start_pos = lexer->position;
  bool seen_dot = false;
  bool seen_digit = false;
  if (lexer->ch == '-') {
    read_char(lexer);
  }

  while (isdigit(lexer->ch) || lexer->ch == '.') {
    if (lexer->ch == '.') {
      if (seen_dot) {
        *ok = false;
      } else {
        seen_dot = true;
      }
    } else {
      seen_digit = true;
    }
    read_char(lexer);
  }

  if (!seen_digit) {
    *ok = false;
  }
  return strndup(lexer->input + start_pos, lexer->position - start_pos);
}

char *read_symbol(lexer_t *lexer) {
  size_t start_pos = lexer->position;
  while (lexer->ch != ' ' && lexer->ch != '\t' && lexer->ch != '\n' &&
         lexer->ch != '\r' && lexer->ch != '(' && lexer->ch != ')' &&
         lexer->ch != '"' && lexer->ch != '\'' && lexer->ch != '@' &&
         lexer->ch != 0) {
    read_char(lexer);
  }
  size_t length = lexer->position - start_pos;
  if (length == 0) {
    return NULL;
  }
  return strndup(lexer->input + start_pos, length);
}

lexer_t lexer_new(const char *input) {
  size_t input_len = strlen(input);
  // clang-format off
  lexer_t lexer = {
    .position = 0,
    .read_position = 0,
    .ch = 0,
    .input = input,
    .input_len = input_len
  };

  read_char(&lexer);
  return lexer;
}

token_t lexer_next_token(lexer_t *lexer) {
  bool ok;
  char *literal;
  skip_whitespace(lexer);
  if (lexer->ch == ';') {
    skip_line_comment(lexer);
  }

  token_t token = {0};
  switch (lexer->ch) {
    case 0: 
      token = token_new(TOKEN_EOF, "");
      break;
    case '-':
      if (isdigit(peek(lexer)) || peek(lexer) == '.') {
        literal = read_number(lexer, &ok);
        if (ok) {
          token = token_new(TOKEN_NUMBER, literal);
        } else {
          token = token_new(TOKEN_INVALID, literal);
          token.line = lexer->line;
          token.column = lexer->column;
        }
      } else {
        token = token_new(TOKEN_SYMBOL, read_symbol(lexer));
      }
      break;
    case '.': 
      if (isdigit(peek(lexer))) {
        literal = read_number(lexer, &ok);
        if (ok) {
          token = token_new(TOKEN_NUMBER, literal);
        } else {
          token = token_new(TOKEN_INVALID, literal);
          token.line = lexer->line;
          token.column = lexer->column;
        }
      } else {
        token = token_new(TOKEN_DOT, ".");
      }
      break;
    case '(': 
      token = token_new(TOKEN_LPAREN, "(");
      read_char(lexer);
      break;
    case ')':
      token = token_new(TOKEN_RPAREN, ")");
      read_char(lexer);
      break;
    case '\'': 
      token = token_new(TOKEN_QUOTE, "'");
      read_char(lexer);
      break;
    case '`': 
      token = token_new(TOKEN_QUASIQUOTE, "`");
      read_char(lexer);
      break;
    case ',':
      if (peek(lexer) == '@') {
        read_char(lexer);
        token = token_new(TOKEN_UNQUOTE_SPLICING, ",@");
      } else {
        token = token_new(TOKEN_UNQUOTE, ",");
      }
      read_char(lexer);
      break;
    case '@':
      token = token_new(TOKEN_INVALID, "@");
      token.line = lexer->line;
      token.column = lexer->column;
      read_char(lexer);
      break;
    case '#': 
      if (peek(lexer) == 't') {
        read_char(lexer);
        read_char(lexer);
        token = token_new(TOKEN_TRUE, "#t");
      } else if (peek(lexer) == 'f') {
        read_char(lexer);
        read_char(lexer);
        token = token_new(TOKEN_FALSE, "#f");
      } else {
        token = token_new(TOKEN_INVALID, read_symbol(lexer));
        token.line = lexer->line;
        token.column = lexer->column;
      }
      break;
    case '"': 
      literal = read_string(lexer, &ok);
      if (ok) {
        token = token_new(TOKEN_STRING, literal);
      } else {
        token = token_new(TOKEN_INVALID, literal);
        token.line = lexer->line;
        token.column = lexer->column;
      }
      break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': 
        literal = read_number(lexer, &ok);
        if (ok) {
          token = token_new(TOKEN_NUMBER, literal);
        } else {
          token = token_new(TOKEN_INVALID, literal);
          token.line = lexer->line;
          token.column = lexer->column;
        }
      break;
    default: 
      token = token_new(TOKEN_SYMBOL, read_symbol(lexer));
      break;
  }

  return token;
}

