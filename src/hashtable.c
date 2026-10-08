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

Entry *ht_add(HashTable *hash_table, const char *key, void *data) {
  unsigned long bucket_idx = hash(key) % HT_BUCKETS;

  if (ht_find(hash_table, key) != NULL) {
    return NULL;
  }

  Entry *new_entry = malloc(sizeof(Entry));
  if (new_entry == NULL) {
    return NULL;
  }

  new_entry->identifier = malloc(strlen(key) + 1);
  if (new_entry->identifier == NULL) {
    free(new_entry);
    return NULL;
  }
  strcpy(new_entry->identifier, key);
  new_entry->data = data;

  new_entry->next = hash_table->array[bucket_idx];
  hash_table->array[bucket_idx] = new_entry;

  return new_entry;
}

void ht_dispose(HashTable *hash_table, void (*free_data)(void *)) {
  if (hash_table == NULL) {
    return;
  }

  for (int i = 0; i < HT_BUCKETS; i++) {
    Entry *entry = hash_table->array[i];

    while (entry != NULL) {
      Entry *next_entry = entry->next;

      if (free_data != NULL && entry->data != NULL) {
        free_data(entry->data);
      }

      free(entry->identifier);
      free(entry);

      entry = next_entry;
    }
  }

  free(hash_table);
}