#pragma once

#include "hashtable.h"
#include <stdbool.h>

/**
 * @brief Enum defining the kinds of symbols.
 */
typedef enum SymbolKind_t { SYMBOL_VARIABLE, SYMBOL_FUNCTION } SymbolKind;

/**
 * @brief Enum defining the data types of symbols.
 */
typedef enum DataType_t {
  TYPE_INT,
  TYPE_DOUBLE,
  TYPE_STRING,
  TYPE_UNIT,
  TYPE_UNKNOWN
} DataType;

/**
 * @brief A structure for storing a symbol.
 *
 * The name and both parameter arrays are freed by symtable.
 */
typedef struct Symbol_t {
  SymbolKind kind; /**< The kind of the symbol. */
  DataType type; /**< The type of the variable, return type of the function. */
  char *name;    /**< The name of the symbol. */

  bool var_modifiable; /**< Is variable modifiable (variables only). */
  int var_unique_id;   /**< The unique id of the variable (variables only). */

  int param_count;       /**< The number of parameters in the function
                            (functions only). */
  DataType *param_types; /**< The data types of the parameters in the function
                            (functions only). */
  char **param_names;    /**< The names of the parameters of the function
                            (functions only). */

} Symbol;

/**
 * @brief A structure for storing the node of a scope.
 *
 * A node of a linked list.
 * outer is NULL for the global scope.
 */
typedef struct SymScope_t {
  HashTable *table;         /**< A pointer to the hash table of the scope. */
  struct SymScope_t *outer; /**< A pointer to the node of the outer scope. */
} SymScope;

/**
 * @brief A structure for storing the symbol table.
 *
 * Everything inside is owned by symtable.
 */
typedef struct SymTable_t {
  SymScope *current_scope; /**< A pointer to the node of the current scope. */
  int var_counter;         /**< The number of defined variables. */
} SymTable;

/**
 * @brief Initializes the symbol table.
 *
 * Allocates space for the symbol table, creates the global scope,
 * and sets var_counter to 0.
 *
 * @return A pointer to the symbol table, NULL if initialization failed.
 */
SymTable *st_init(void);

/**
 * @brief Frees the symbol table and its contents.
 *
 * Also closes every open scope and frees all symbols.
 *
 * @param symtable The symbol table to be freed.
 */
void st_dispose(SymTable *symtable);