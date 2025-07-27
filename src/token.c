#include "token.h"
#include <stdlib.h>
#include <string.h>

token_t token_new(token_type_t type, const char *literal) {
  token_t token;
  token.type = type;
  if (literal) {
    token.literal = strdup(literal);
  } else {
    token.literal = NULL;
  }

  return token;
}

void token_free(token_t *token) {
  if (token->literal) {
    free(token->literal);
    token->literal = NULL;
  }
}
