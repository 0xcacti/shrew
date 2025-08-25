#include "builtin.h"
#include "env.h"
#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include "symbol.h"
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int read_file(const char *path, char **script_contents);
void repl(env_t *env);
static volatile sig_atomic_t should_exit = 0;

static void signal_handler(int sig) {
  (void)sig;
  should_exit = 1;
  write(STDOUT_FILENO, "\n", 1);
}

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
  bool interactive = false;
  char *script_path = NULL;
  char *script_contents = NULL;
  env_t env = { 0 };
  lexer_t lexer = { 0 };
  parser_t parser = { 0 };

  symbol_intern_init();
  bool s = env_init(&env, NULL);
  if (!s) {
    fprintf(stderr, "Failed to initialize environment\n");
    return 1;
  }
  env_add_builtins(&env);

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
      interactive = true;
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

  script_path = script_path ? script_path : argv[optind];

  if (script_path) {
    if (read_file(script_path, &script_contents) != 0) {
      env_destroy(&env);
      symbol_intern_free_all();
      return 1;
    }
  }

  if (script_contents) {
    lexer = lexer_new(script_contents);
    parser = parser_new(&lexer);
    parse_result_t parse_result = parser_parse(&parser);
    if (parser.error_count > 0) {
      for (size_t i = 0; i < parser.error_count; i++) {
        fprintf(stderr, "Parse error: %s\n", parser.errors[i]);
      }
      parser_free(&parser);
      free(script_contents);
      env_destroy(&env);
      symbol_intern_free_all();
      return 1;
    }
    if (parse_result.expressions == NULL) {
      fprintf(stderr, "No expressions to evaluate\n");
      parser_free(&parser);
      free(script_contents);
      env_destroy(&env);
      symbol_intern_free_all();
      return 1;
    }

    eval_result_t eval_result = evaluate_many(parse_result.expressions, parse_result.count, &env);
    if (eval_result.status != EVAL_OK) {
      fprintf(stderr, "Evaluation error: %s\n", eval_result.error_message);
      evaluator_result_free(&eval_result);
      parser_free(&parser);
      free(script_contents);
      env_destroy(&env);
      symbol_intern_free_all();
      return 1;
    }

    lval_print(eval_result.result);
    printf("\n");
  } else {
    interactive = true;
  }

  if (interactive) {
    printf("Welcome to the Shrew REPL!\n");
    repl(&env);
  }
  env_destroy(&env);
  symbol_intern_free_all();

  return 0;
}

void repl(env_t *env) {
  char *line = NULL;
  size_t len = 0;

  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  while (!should_exit) {
    printf("-> ");
    fflush(stdout);

    ssize_t read = getline(&line, &len, stdin);
    if (read == -1) {
      if (should_exit) {
        break;
      }
      if (errno == EINTR) {
        continue;
      }
      printf("\n");
      break;
    }

    lexer_t lexer = lexer_new(line);
    parser_t parser = parser_new(&lexer);
    parse_result_t parse_result = parser_parse(&parser);
    if (parser.error_count > 0) {
      for (size_t i = 0; i < parser.error_count; i++) {
        fprintf(stderr, "Parse error: %s\n", parser.errors[i]);
      }
      parser_free(&parser);
      continue;
    }
    if (parse_result.expressions == NULL) {
      parser_free(&parser);
      continue;
    }
    eval_result_t eval_result = evaluate_single(parse_result.expressions[0], env);
    if (eval_result.status != EVAL_OK) {
      fprintf(stderr, "Evaluation error: %s\n", eval_result.error_message);
      evaluator_result_free(&eval_result);
      parser_free(&parser);
      continue;
    }
    lval_print(eval_result.result);
    printf("\n");
  }
  if (line) {
    free(line);
  }
}

int read_file(const char *path, char **script_contents) {
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("Error opening script file");
    return 1;
  }
  struct stat dbstat = { 0 };
  fstat(fd, &dbstat);
  size_t size = dbstat.st_size;
  *script_contents = malloc(size + 1);
  if (!*script_contents) {
    perror("malloc");
    close(fd);
    return 1;
  }

  if (read(fd, *script_contents, size) != (ssize_t)size) {
    perror("read");
    free(*script_contents);
    *script_contents = NULL;
    close(fd);
    return 1;
  }
  (*script_contents)[size] = '\0';
  close(fd);
  return 0;
}
