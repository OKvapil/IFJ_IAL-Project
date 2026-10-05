#include "hashtable.h"
#include <stdlib.h>

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