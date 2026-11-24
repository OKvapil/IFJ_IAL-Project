/**
 * @file hashtable.c
 * @brief The implementation of the hash table.
 * 
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Jonáš Ľalík (xlalikj00)
 */

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

static void get_index_and_step(unsigned int table_size, const char *key,
                               unsigned long *first, unsigned long *step) {
  unsigned long h = hash(key);
  *first = h % table_size;
  *step = 1 + h % (table_size - 1);
}

Entry *ht_find(HashTable *hash_table, const char *key) {
  unsigned long first;
  unsigned long step;
  get_index_and_step(hash_table->size, key, &first, &step);

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
static int find_free_slot(Entry **array, unsigned int size, const char *key,
                          unsigned long *slot_idx) {
  unsigned long first;
  unsigned long step;
  get_index_and_step(size, key, &first, &step);

  *slot_idx = first;
  for (unsigned int i = 0; i < size; i++) {
    if (array[*slot_idx] == NULL) {
      return 0;
    }

    *slot_idx = (*slot_idx + step) % size;
  }

  return -1;
}

static int ht_resize(HashTable *hash_table) {
  unsigned int new_size = hash_table->size;
  for (unsigned int i = 0; i < sizeof(primes) / sizeof(primes[0]); i++) {
    if (primes[i] > hash_table->size) {
      new_size = primes[i];
      break;
    }
  }

  if (new_size == hash_table->size) {
    return -1;
  }

  Entry **new_array = malloc(new_size * sizeof(Entry *));
  if (new_array == NULL) {
    return -1;
  }

  for (unsigned int i = 0; i < new_size; i++) {
    new_array[i] = NULL;
  }

  for (unsigned int i = 0; i < hash_table->size; i++) {
    if (hash_table->array[i] != NULL) {

      unsigned long slot_idx;
      if (find_free_slot(new_array, new_size, hash_table->array[i]->identifier,
                         &slot_idx) == -1) {
        free(new_array);
        return -1;
      }

      new_array[slot_idx] = hash_table->array[i];
    }
  }

  free(hash_table->array);
  hash_table->array = new_array;
  hash_table->size = new_size;

  return 0;
}

Entry *ht_add(HashTable *hash_table, const char *key, void *data) {
  unsigned long slot_idx;

  if (ht_find(hash_table, key) != NULL) {
    return NULL;
  }

  if ((hash_table->count + 1) * 2 > hash_table->size) {
    if (ht_resize(hash_table) == -1) {
      return NULL;
    }
  }

  if (find_free_slot(hash_table->array, hash_table->size, key, &slot_idx) ==
      -1) {
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