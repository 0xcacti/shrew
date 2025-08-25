#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include <getopt.h>
#include <stdio.h>

static void print_usage(const char *progname) {
  fprintf(stderr, "Usage: %s [options] [path]\n", progname);
  fprintf(stderr, "Arguments:\n");
  fprintf(stderr, "  [PATH]             Path to script file to execute\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -i, --interactive  Start in after executing script interactive mode (REPL)\n");
  fprintf(stderr, "  -h, --help         Show this help message and exit\n");
}

int main(int argc, char *argv[]) {
  int opt;
  char *script_path = NULL;

  // clang-format off
  static struct option long_options[] = {
    {"interactive", no_argument, 0, 'i'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  // clang-format on

  while ((opt = getopt_long(argc, argv, "i:h", long_options, NULL)) != -1) {
    switch (opt) {
    case 'i':
      printf("testing interactive mode\n");
      script_path = optarg;
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  if (script_path) {
    printf("Executing script: %s\n", script_path);
  }

  return 0;
}
