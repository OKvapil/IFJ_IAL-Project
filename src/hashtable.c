#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

HashTable *ht_init(void) {
  HashTable *new_table = malloc(sizeof(HashTable));

  if (new_table == NULL) {
    return NULL;
  }

  for (int i = 0; i < HT_BUCKETS; i++) {
    new_table->array[i] = NULL;
  }

  return new_table;
}

// djb2 hash function
static unsigned long hash(const char *str) {
  unsigned long h = 5381;
  int c;

  while ((c = *str++)) {
    h = ((h << 5) + h) + c;
  }

  return h;
}

Entry *ht_find(HashTable *hash_table, const char *key) {
  unsigned long bucket_idx = hash(key) % HT_BUCKETS;

  Entry *entry = hash_table->array[bucket_idx];

  while (entry != NULL) {
    if (!strcmp(entry->identifier, key)) {
      return entry;
    }

    entry = entry->next;
  }

  return NULL;
}