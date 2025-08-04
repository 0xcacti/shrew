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

void skip_noise(lexer_t *lexer) {
  for (;;) {
    skip_whitespace(lexer);
    if (lexer->ch == ';') {
      skip_line_comment(lexer);
      continue;
    }
    break;
  }
}

char *read_string(lexer_t *lexer, bool *ok) {
  read_char(lexer);

  size_t cap = 16, len = 0;
  char *buf = malloc(cap);
  if (!buf) {
    *ok = false;
    return NULL;
  }

  bool terminated = false;

  while (lexer->ch != 0) {
    if (lexer->ch == '"') {
      terminated = true;
      break;
    }

    if (lexer->ch == '\\') {
      read_char(lexer);

      char translated;
      bool recognised = true;
      switch (lexer->ch) {
      case 'n':
        translated = '\n';
        break;
      case 't':
        translated = '\t';
        break;
      case 'r':
        translated = '\r';
        break;
      case '"':
        translated = '"';
        break;
      case '\\':
        translated = '\\';
        break;
      case '0':
        translated = '\0';
        break;
      default:
        recognised = false;
      }

      if (recognised) {

        if (len + 1 >= cap) {
          cap *= 2;
          buf = realloc(buf, cap);
        }
        if (!buf) {
          *ok = false;
          return NULL;
        }
        buf[len++] = translated;
        read_char(lexer);
        continue;
      }

      if (len + 2 >= cap) {
        cap *= 2;
        buf = realloc(buf, cap);
      }
      if (!buf) {
        *ok = false;
        return NULL;
      }
      buf[len++] = '\\';
      buf[len++] = lexer->ch;
      read_char(lexer);
      continue;
    }

    if (len + 1 >= cap) {
      cap *= 2;
      buf = realloc(buf, cap);
    }
    if (!buf) {
      *ok = false;
      return NULL;
    }
    buf[len++] = lexer->ch;
    read_char(lexer);
  }

  buf[len] = '\0';

  if (terminated) {
    *ok = true;
    read_char(lexer);
  } else {
    *ok = false;
  }

  return buf;
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
    .input_len = input_len,
    .line = 1,
    .column = 0
  };
  // clang-format on

  read_char(&lexer);
  return lexer;
}

token_t lexer_next_token(lexer_t *lexer) {
  bool ok;
  char *literal;
  skip_noise(lexer);

  size_t start_line = lexer->line;
  size_t start_column = lexer->column; /* column already points at `ch` */

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
      }
    } else {
      token = token_new(TOKEN_DOT, ".");
      read_char(lexer);
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
    }
    break;
  case '"':
    literal = read_string(lexer, &ok);
    if (ok) {
      token = token_new(TOKEN_STRING, literal);
    } else {
      token = token_new(TOKEN_INVALID, literal);
    }
    break;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    literal = read_number(lexer, &ok);
    if (ok) {
      token = token_new(TOKEN_NUMBER, literal);
    } else {
      token = token_new(TOKEN_INVALID, literal);
    }
    break;
  default:
    token = token_new(TOKEN_SYMBOL, read_symbol(lexer));
    break;
  }
  token.line = start_line;
  token.column = start_column;

  return token;
}
