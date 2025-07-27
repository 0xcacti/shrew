#include "lexer.h"
#include <criterion/criterion.h>

Test(lexer_tests, creation) {
  const char *input = "(+ 1 2)";
  lexer_t lexer = lexer_new(input);

  cr_assert_eq(lexer.position, 0, "position should start at 0");
  cr_assert_eq(lexer.read_position, 1, "read_position should start at 1");
  cr_assert_eq(lexer.ch, '(', "first character should be (");
}
