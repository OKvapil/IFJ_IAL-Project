#pragma once

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
  DataType
      type; /**< The data type of the variable / return type of the function. */
  char *name; /**< The name of the symbol. */

  bool var_modifiable; /**< States whether the variable is modifiable (variables
                          only). */
  int var_unique_id; /**< The global unique id of the variable (variables only).
                      */

  int param_count; /**< The number of parameters in the function (functions
                      only). */
  DataType *param_types; /**< The data types of the parameters in the function
                            (functions only). */
  char **param_names;    /**< The names of the parameters of the function
                            (functions only). */

} Symbol;