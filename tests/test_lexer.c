#include "lexer.h"
#include <criterion/criterion.h>

Test(lexer, simple_tokenization) {
  const char *input = "(+ 1 2)";
  struct token_list *tokens = tokenize(input);
}
