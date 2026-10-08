#pragma once

/** The number of buckets in the hash table. */
#define HT_BUCKETS 101

/**
 * @brief A structure storing one hash table entry.
 */
typedef struct Entry_t {
  char *identifier;       /**< The identifier of the entry. */
  void *data;             /**< An opaque pointer to the data of the entry. */
  struct Entry_t *next;   /**< The pointer to the next entry in the same bucket, NULL at the end of the chain. */
} Entry;

/**
 * @brief A structure for storing a hash table.
 */
typedef struct HashTable_t {
  Entry *array[HT_BUCKETS];   /**< The array of the entry buckets. */
} HashTable;


/**
 * @brief Initializes the hash table.
 * 
 * Allocates space for the hash table and initializes all buckets to NULL.
 * 
 * @return A pointer to the initialized hash table, NULL if initialization failed.
 */
HashTable *ht_init(void);

/**
 * @brief Finds an entry by its key.
 * 
 * Uses the djb2 hash function.
 * 
 * @param hash_table The hash table that the key is searched in.
 * @param key The key that is searched.
 * @return A pointer to the found entry, NULL if the entry was not found.
 */
Entry *ht_find(HashTable *hash_table, const char *key);

/**
 * @brief Adds an entry to the hash table.
 * 
 * Uses the djb2 hash function.
 * Adds the entry to the start of the chain.
 * The key is copied, caller keeps ownership of the string.
 * 
 * @param hash_table The hash table in which the entry is added.
 * @param key The key of the entry that is added.
 * @param data The data of the entry that is added.
 * @return A pointer to the added entry, NULL if duplicate or allocation error.
 */
Entry *ht_add(HashTable *hash_table, const char *key, void *data);

/**
 * @brief Frees the hash table and its contents.
 * 
 * Also frees the data of the entries via free_data.
 * 
 * @param hash_table The hash table to be freed.
 * @param free_data A pointer to the function which frees the data of the entries. If NULL, the data is not freed.
 */
void ht_dispose(HashTable *hash_table, void (*free_data)(void *));