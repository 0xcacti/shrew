#include "hashtable.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static hashtable symbol_table;

void symbol_intern_init(void) {
  ht_error err = { 0 };
  ht_init(&symbol_table, 128, &err);
}

const char *symbol_intern(const char *name) {
  if (!name) return NULL;

  void *out = NULL;
  if (ht_get(&symbol_table, name, &out)) {
    return (const char *)out;
  }

  size_t len = strlen(name);
  char *interned_name = malloc(len + 1);
  if (!interned_name) {
    fprintf(stderr, "symbol_intern: out of memory copying '%s'\n", name);
    return NULL;
  }
  strcpy(interned_name, name);

  ht_error err = { 0 };
  if (!ht_set(&symbol_table, interned_name, interned_name, &err)) {
    fprintf(stderr,
            "symbol_intern: hashtable insert failed: %s\n",
            err.error_message ? err.error_message : "(unknown)");
    free(interned_name);
    return NULL;
  }
  return interned_name;
}

void symbol_intern_free_all(void) {
  ht_destroy(&symbol_table);
  symbol_table = (hashtable){ 0 };
}
