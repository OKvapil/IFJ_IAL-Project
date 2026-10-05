#pragma once

#define HT_BUCKETS 101

typedef struct Entry_t {
  char *identifier;
  void *data;
  struct Entry_t *next;
} Entry;

typedef struct HashTable_t {
  Entry *array[HT_BUCKETS];
} HashTable;

HashTable *ht_init(void);

Entry *ht_find(HashTable *hash_table, const char *key);

Entry *ht_add(HashTable *hash_table, const char *key, void *data);

// ht_dispose also frees the entry data
void ht_dispose(HashTable *hash_table, void (*free_data)(void *));