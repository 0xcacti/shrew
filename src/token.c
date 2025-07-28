#include "token.h"
#include <stdlib.h>
#include <string.h>

token_t token_new(token_type_t type, const char *literal) {
  token_t token = {0};
  token.type = type;
  if (literal) {
    token.literal = strdup(literal);
  } else {
    token.literal = NULL;
  }
  token.line = 0;
  token.column = 0;

  return token;
}

void token_free(token_t *token) {
  if (token->literal) {
    free(token->literal);
    token->literal = NULL;
  }
}
