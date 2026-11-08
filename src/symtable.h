/**
 * @file symtable.h
 * @brief Symbol table with scope handling for the semantic analysis.
 * 
 * Built on top of the hash table, one hash table per scope.
 * 
 * IFJ/IAL project - IFJ26 compiler
 * 
 * @author Jonáš Ľalík (xlalikj00)
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "hashtable.h"
#include "types.h"
#include <stdbool.h>

/**
 * @brief Enum defining the kinds of symbols.
 */
typedef enum SymbolKind_t { SYMBOL_VARIABLE, SYMBOL_FUNCTION } SymbolKind;

/**
 * @brief Enum defining the return values of the variable/function declaration
 * functions.
 */
typedef enum DeclarationCode_t {
  DECL_OK,
  DECL_REDEF,
  DECL_ALLOC_FAIL
} DeclarationCode;

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

/**
 * @brief Enters an inner scope of the code.
 *
 * Allocates space for a new scope.
 * The new scope becomes the current one.
 *
 * @param symtable The symbol table in which to enter a deeper scope.
 * @return 0 if successful, -1 if allocation failure.
 */
int st_enter_scope(SymTable *symtable);

/**
 * @brief Exits to the outer scope of the code.
 *
 * Frees the scope being left together with its symbols.
 * If it's the global scope, the symbol table is unchanged.
 *
 * @param symtable The symbol table in which to exit to the outer scope.
 * @return 0 if successful, -1 if it's the global scope.
 */
int st_exit_scope(SymTable *symtable);

/**
 * @brief Declares a variable symbol.
 *
 * Declares the variable in the current scope (shadowing enabled), copies the
 * name, takes the next id from the counter and increments it. new_symbol is
 * written only on success and belongs to symtable. new_symbol can be NULL if
 * the caller doesn't need it.
 *
 * @param symtable The symbol table in which the variable goes.
 * @param name The name of the variable.
 * @param type The data type of the variable.
 * @param modifiable Is the variable modifiable.
 * @param new_symbol The newly created variable symbol.
 * @return DECL_OK if successful, DECL_REDEF if redefinition, DECL_ALLOC_FAIL if
 * allocation failure.
 */
DeclarationCode st_declare_var(SymTable *symtable, const char *name,
                               DataType type, bool modifiable,
                               Symbol **new_symbol);

/**
 * @brief Declares a function symbol.
 *
 * Declares the function in the global scope.
 * new_symbol is written only on success and belongs to symtable.
 * new_symbol can be NULL if the caller doesn't need it.
 * type may be TYPE_UNKNOWN when the return type is inferred.
 * Parameter arrays and names inside param_names are copied, caller keeps
 * ownership.
 *
 * @param symtable The symbol table in which the function goes.
 * @param name The name of the function.
 * @param type The return type of the function.
 * @param param_count The number of parameters of the function.
 * @param param_types An array of the parameter data types.
 * @param param_names An array of the parameter names.
 * @param new_symbol The newly created function symbol.
 * @return DECL_OK if successful, DECL_REDEF if redefinition, DECL_ALLOC_FAIL if
 * allocation failure.
 */
DeclarationCode st_declare_fun(SymTable *symtable, const char *name,
                               DataType type, int param_count,
                               const DataType *param_types,
                               const char *const *param_names,
                               Symbol **new_symbol);

/**
 * @brief Looks up a symbol in the symbol table.
 *
 * The search starts in the current scope and moves outward.
 * Variables and functions share one namespace, kind has to be checked.
 * The returned symbol belongs to symtable and is freed on scope exit.
 *
 * @param symtable The symbol table to search in.
 * @param name The name of the symbol to search for.
 * @return A pointer to the found symbol, NULL if not found.
 */
Symbol *st_lookup(const SymTable *symtable, const char *name);

#endif // SYMTABLE_H