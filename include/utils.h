#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(HT_STRING_KEYS) && !defined(HT_POINTER_KEYS) 
#define HT_STRING_KEYS 1
#endif

#if defined(HT_STRING_KEYS) && defined(HT_POINTER_KEYS)
#error "Define only one of HT_STRING_KEYS or HT_POINTER_KEYS."
#endif

typedef struct {
#if HT_STRING_KEYS
  const char *key;
#else 
  const void *key;
#endif 
  void *value;
  uint64_t hash;
  uint32_t dib;
} ht_entry;

typedef struct {
  ht_entry *entries;
  size_t capacity;
  size_t size;
  size_t tombstones;
} hashtable;

typedef struct {
  const char *error_message;
} ht_error;

// Lifecycle
bool ht_init(hashtable *table, size_t initial_capacity, ht_error *err);
void ht_destroy(hashtable *table);

// Operations
bool ht_set(hashtable *table,
#if HT_STRING_KEYS
 const char *key,
#else
 const void *key,
#endif
 void *value,
 ht_error *err);

bool ht_get(const hashtable *table,
#if HT_STRING_KEYS
 const char *key,
#else
  const void *key,
#endif
  void **out_value);

bool ht_erase(hashtable *table,
#if HT_STRING_KEYS
  const char *key,
#else
  const void *key,
#endif
  void **out_old_value);

// Introspection
static inline size_t ht_count(const hashtable *t) { return t->size; }
static inline size_t ht_capacity(const hashtable *t) { return t->capacity; }

// Iteration
typedef struct {
    const hashtable *table;
    size_t           index;
} ht_iter;

void ht_iter_begin(const hashtable *table, ht_iter *it);
bool ht_iter_next(ht_iter *it, 
#if HT_STRING_KEYS
                  const char **out_key,
#else
                  const void **out_key,
#endif
                  void **out_value);

#endif // UTILS_H
