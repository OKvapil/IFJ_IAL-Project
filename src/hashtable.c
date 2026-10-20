#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

static const unsigned int primes[10] = {101,  211,  431,   863,   1741,
                                        3491, 6983, 13967, 27941, 55889};

HashTable *ht_init(void) {
  HashTable *new_table = malloc(sizeof(HashTable));
  if (new_table == NULL) {
    return NULL;
  }

  new_table->size = primes[0];

  new_table->array = malloc(new_table->size * sizeof(Entry *));
  if (new_table->array == NULL) {
    free(new_table);
    return NULL;
  }

  new_table->count = 0;
  for (unsigned int i = 0; i < new_table->size; i++) {
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

static void get_index_and_step(HashTable *hash_table, const char *key,
                               unsigned long *first, unsigned long *step) {
  unsigned long h = hash(key);
  *first = h % hash_table->size;
  *step = 1 + h % (hash_table->size - 1);
}

Entry *ht_find(HashTable *hash_table, const char *key) {
  unsigned long first;
  unsigned long step;
  get_index_and_step(hash_table, key, &first, &step);

  unsigned long slot_idx = first;
  for (unsigned int i = 0; i < hash_table->size; i++) {
    if (hash_table->array[slot_idx] == NULL) {
      return NULL;
    }

    if (strcmp(hash_table->array[slot_idx]->identifier, key) == 0) {
      return hash_table->array[slot_idx];
    }

    slot_idx = (slot_idx + step) % hash_table->size;
  }

  return NULL;
}

// return 0 when found, return -1 when no free slot
static int find_free_slot(HashTable *hash_table, const char *key,
                          unsigned long *slot_idx) {
  unsigned long first;
  unsigned long step;
  get_index_and_step(hash_table, key, &first, &step);

  *slot_idx = first;
  for (unsigned int i = 0; i < hash_table->size; i++) {
    if (hash_table->array[*slot_idx] == NULL) {
      return 0;
    }

    *slot_idx = (*slot_idx + step) % hash_table->size;
  }

  return -1;
}

Entry *ht_add(HashTable *hash_table, const char *key, void *data) {
  unsigned long slot_idx;

  if (ht_find(hash_table, key) != NULL) {
    return NULL;
  }

  if (find_free_slot(hash_table, key, &slot_idx) == -1) {
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

  hash_table->array[slot_idx] = new_entry;
  hash_table->count++;

  return new_entry;
}

void ht_dispose(HashTable *hash_table, void (*free_data)(void *)) {
  if (hash_table == NULL) {
    return;
  }

  for (unsigned int i = 0; i < hash_table->size; i++) {
    Entry *entry = hash_table->array[i];
    if (entry == NULL) {
      continue;
    }

    if (free_data != NULL && entry->data != NULL) {
      free_data(entry->data);
    }

    free(entry->identifier);
    free(entry);
  }

  free(hash_table->array);
  free(hash_table);
}