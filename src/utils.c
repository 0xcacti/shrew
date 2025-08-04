#include "utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef HT_MAX_LOAD_NUM
#define HT_MAX_LOAD_NUM 80 // load factor numerator -> 0.80
#endif
#ifndef HT_MAX_LOAD_DEN
#define HT_MAX_LOAD_DEN 100
#endif

// Sentinel for tombstones: unique non-NULL pointer unlikely to collide with
// real keys
static const void *const HT_TOMBSTONE =
    (const void *)(uintptr_t)(~(uintptr_t)0ULL - 1ULL);

// SplitMix64 for pointer hashing or 64-bit mix
static uint64_t splitmix64(uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  x = x ^ (x >> 31);
  return x;
}

#if HT_STRING_KEYS
// FNV-1a 64-bit for strings
static uint64_t hash_cstr(const char *s) {
  uint64_t h = 1469598103934665603ULL;
  while (*s) {
    h ^= (unsigned char)(*s++);
    h *= 1099511628211ULL;
  }
  // final avalanching
  return splitmix64(h);
}
#else
static uint64_t hash_ptr(const void *p) {
  return splitmix64((uint64_t)(uintptr_t)p);
}
#endif

// Key equality and duplication
#if HT_STRING_KEYS
static inline bool keys_equal(const char *a, const char *b) {
  return strcmp(a, b) == 0;
}
#ifdef HT_DUP_KEYS
static inline const char *dup_key(const char *k) {
  size_t n = strlen(k) + 1;
  char *copy = (char *)malloc(n);
  if (copy)
    memcpy(copy, k, n);
  return copy;
}
#else
static inline const char *dup_key(const char *k) { return k; }
#endif
#else
static inline bool keys_equal(const void *a, const void *b) { return a == b; }
#endif

static inline size_t next_power_of_two(size_t x) {
  if (x < 8)
    return 8;
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  return x + 1;
}

static bool ht_resize(hashtable *table, size_t new_capacity, ht_error *err);

bool ht_init(hashtable *table, size_t initial_capacity, ht_error *err) {
  if (!table)
    return false;
  size_t cap = next_power_of_two(initial_capacity ? initial_capacity : 16);
  table->entries = (ht_entry *)calloc(cap, sizeof(ht_entry));
  if (!table->entries) {
    if (err)
      err->error_message = "Out of memory allocating entries.";
    return false;
  }
  table->capacity = cap;
  table->size = 0;
  table->tombstones = 0;
  return true;
}

void ht_destroy(hashtable *table) {
  if (!table || !table->entries)
    return;
#if HT_STRING_KEYS && defined(HT_DUP_KEYS)
  for (size_t i = 0; i < table->capacity; ++i) {
    const void *k = table->entries[i].key;
    if (k && k != HT_TOMBSTONE) {
      free((void *)table->entries[i].key);
    }
  }
#endif
  free(table->entries);
  table->entries = NULL;
  table->capacity = table->size = table->tombstones = 0;
}

static inline bool should_grow(const hashtable *t) {
  size_t used = t->size + t->tombstones;
  // grow when (used / capacity) > load factor
  return (used * HT_MAX_LOAD_DEN) > (t->capacity * HT_MAX_LOAD_NUM);
}

bool ht_set(hashtable *table,
#if HT_STRING_KEYS
            const char *key,
#else
            const void *key,
#endif
            void *value, ht_error *err) {
  assert(table && table->entries);
  if (should_grow(table)) {
    if (!ht_resize(table, table->capacity * 2, err))
      return false;
  }

#if HT_STRING_KEYS
  uint64_t h = hash_cstr(key);
#else
  uint64_t h = hash_ptr(key);
#endif
  size_t mask = table->capacity - 1;
  size_t index = (size_t)(h & mask);

  ht_entry entry = {
#if HT_STRING_KEYS
      .key = dup_key(key),
#else
      .key = key,
#endif
      .value = value,
      .hash = h,
      .dib = 1};

#if HT_STRING_KEYS && defined(HT_DUP_KEYS)
  if (!entry.key) {
    if (err)
      err->error_message = "Out of memory duplicating key.";
    return false;
  }
#endif

  for (;;) {
    ht_entry *slot = &table->entries[index];
    if (slot->key == NULL) {
      *slot = entry;
      table->size += 1;
      return true;
    }
    if (slot->key == HT_TOMBSTONE) {
      *slot = entry;
      table->size += 1;
      table->tombstones -= 1;
      return true;
    }
    if (slot->hash == entry.hash && keys_equal(slot->key,
#if HT_STRING_KEYS
                                               key
#else
                                               key
#endif
                                               )) {
      // Replace value, keep key
#if HT_STRING_KEYS && defined(HT_DUP_KEYS)
      free((void *)entry.key); // we duplicated; discard duplicate
#endif
      slot->value = value;
      return true;
    }
    if (slot->dib < entry.dib) {
      // Robin Hood swap
      ht_entry tmp = *slot;
      *slot = entry;
      entry = tmp;
    }
    index = (index + 1) & mask;
    entry.dib += 1;
  }
}

bool ht_get(const hashtable *table,
#if HT_STRING_KEYS
            const char *key,
#else
            const void *key,
#endif
            void **out_value) {
  assert(table && table->entries);
#if HT_STRING_KEYS
  uint64_t h = hash_cstr(key);
#else
  uint64_t h = hash_ptr(key);
#endif
  size_t mask = table->capacity - 1;
  size_t index = (size_t)(h & mask);
  uint32_t dib = 1;

  for (;;) {
    const ht_entry *slot = &table->entries[index];
    if (slot->key == NULL)
      return false; // not found
    if (slot->key != HT_TOMBSTONE) {
      if (slot->hash == h && keys_equal(slot->key, key)) {
        if (out_value)
          *out_value = slot->value;
        return true;
      }
      if (slot->dib < dib)
        return false; // early exit
    }
    index = (index + 1) & mask;
    dib += 1;
  }
}

bool ht_erase(hashtable *table,
#if HT_STRING_KEYS
              const char *key,
#else
              const void *key,
#endif
              void **out_old_value) {
  assert(table && table->entries);
#if HT_STRING_KEYS
  uint64_t h = hash_cstr(key);
#else
  uint64_t h = hash_ptr(key);
#endif
  size_t mask = table->capacity - 1;
  size_t index = (size_t)(h & mask);
  uint32_t dib = 1;

  for (;;) {
    ht_entry *slot = &table->entries[index];
    if (slot->key == NULL)
      return false;
    if (slot->key != HT_TOMBSTONE) {
      if (slot->hash == h && keys_equal(slot->key, key)) {
        if (out_old_value)
          *out_old_value = slot->value;
#if HT_STRING_KEYS && defined(HT_DUP_KEYS)
        free((void *)slot->key);
#endif
        slot->key = HT_TOMBSTONE;
        slot->value = NULL;
        slot->hash = 0;
        slot->dib = 0;
        table->size -= 1;
        table->tombstones += 1;
        return true;
      }
      if (slot->dib < dib)
        return false;
    }
    index = (index + 1) & mask;
    dib += 1;
  }
}

static bool ht_resize(hashtable *table, size_t new_capacity, ht_error *err) {
  new_capacity = next_power_of_two(new_capacity);
  ht_entry *old_entries = table->entries;
  size_t old_capacity = table->capacity;

  ht_entry *new_entries = (ht_entry *)calloc(new_capacity, sizeof(ht_entry));
  if (!new_entries) {
    if (err)
      err->error_message = "Out of memory resizing table.";
    return false;
  }

  table->entries = new_entries;
  table->capacity = new_capacity;
  table->size = 0;
  table->tombstones = 0;

  size_t mask = new_capacity - 1;
  for (size_t i = 0; i < old_capacity; ++i) {
    ht_entry e = old_entries[i];
    if (!e.key || e.key == HT_TOMBSTONE)
      continue;

    size_t index = (size_t)(e.hash & mask);
    e.dib = 1;
    for (;;) {
      ht_entry *slot = &new_entries[index];
      if (slot->key == NULL) {
        *slot = e;
        break;
      }
      if (slot->dib < e.dib) {
        ht_entry tmp = *slot;
        *slot = e;
        e = tmp;
      }
      index = (index + 1) & mask;
      e.dib += 1;
    }
    table->size += 1;
  }

  free(old_entries);
  return true;
}

void ht_iter_begin(const hashtable *table, ht_iter *it) {
  it->table = table;
  it->index = 0;
}

bool ht_iter_next(ht_iter *it,
#if HT_STRING_KEYS
                  const char **out_key,
#else
                  const void **out_key,
#endif
                  void **out_value) {
  while (it->index < it->table->capacity) {
    const ht_entry *slot = &it->table->entries[it->index++];
    if (slot->key && slot->key != HT_TOMBSTONE) {
      if (out_key)
        *out_key = slot->key;
      if (out_value)
        *out_value = slot->value;
      return true;
    }
  }
  return false;
}
