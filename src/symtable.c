/**
 * @file symtable.c
 * @brief Implementation of the symbol table functions
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Jonáš Ľalík (xlalikj00)
 */

#include "symtable.h"
#include <stdlib.h>

/**
 * @brief Frees a symbol and everything it owns.
 *
 * Frees the name, the parameter types array, each parameter name, the
 * parameter names array and the symbol itself. Safe for both variable and
 * function symbols, because unused pointers are NULL.
 * Passed to ht_dispose as the free_data callback.
 *
 * @param data A pointer to the Symbol to be freed.
 */
static void free_symbol(void *data) {
  Symbol *sym = (Symbol *)data;

  free(sym->name);
  free(sym->param_types);

  if (sym->param_names != NULL) {
    for (int i = 0; i < sym->param_count; i++) {
      free(sym->param_names[i]);
    }
  }

  free(sym->param_names);
  free(sym);
}

typedef struct BuiltinFun_t {
  const char *name;
  DataType return_type;
  int param_count;
  DataType param_types[3];
} BuiltinFun;

static const BuiltinFun builtins[] = {
    {"print", TYPE_UNIT, 1, {TYPE_UNKNOWN}}, // CHANGE TO TYPE_ANY
    {"readLine", TYPE_STRING, 0, {0}},
    {"readInt", TYPE_INT, 0, {0}},
    {"readDouble", TYPE_DOUBLE, 0, {0}},
    {"readBoolean", TYPE_BOOL, 0, {0}},
    {"toStr", TYPE_STRING, 1, {TYPE_UNKNOWN}}, // CHANGE TO TYPE_ANY
    {"toInt", TYPE_INT, 1, {TYPE_DOUBLE}},
    {"strLength", TYPE_INT, 1, {TYPE_STRING}},
    {"subStr", TYPE_STRING, 3, {TYPE_STRING, TYPE_INT, TYPE_INT}},
    {"ord", TYPE_INT, 2, {TYPE_STRING, TYPE_INT}},
    {"chr", TYPE_STRING, 1, {TYPE_INT}},
};

SymTable *st_init(void) {
  SymTable *new_symtable = malloc(sizeof(SymTable));
  if (new_symtable == NULL) {
    return NULL;
  }
  new_symtable->var_counter = 0;

  SymScope *global_scope = malloc(sizeof(SymScope));
  if (global_scope == NULL) {
    free(new_symtable);
    return NULL;
  }
  new_symtable->current_scope = global_scope;
  global_scope->outer = NULL;

  HashTable *hash_table = ht_init();
  if (hash_table == NULL) {
    free(global_scope);
    free(new_symtable);
    return NULL;
  }
  global_scope->table = hash_table;

  for (size_t i = 0; i < sizeof(builtins) / sizeof(BuiltinFun); i++) {
    if (st_declare_fun(new_symtable, builtins[i].name, builtins[i].return_type,
                       builtins[i].param_count, builtins[i].param_types, NULL,
                       NULL) != DECL_OK) {
      st_dispose(new_symtable);
      return NULL;
    }
  }

  return new_symtable;
}

/**
 * @brief Frees the current scope, its symbols, and moves to the outer scope.
 *
 * @param symtable The symtable whose current scope to delete.
 */
static void pop_scope(SymTable *symtable) {
  ht_dispose(symtable->current_scope->table, free_symbol);
  SymScope *to_free = symtable->current_scope;
  symtable->current_scope = symtable->current_scope->outer;
  free(to_free);
}

void st_dispose(SymTable *symtable) {
  if (symtable == NULL) {
    return;
  }

  while (symtable->current_scope != NULL) {
    pop_scope(symtable);
  }

  free(symtable);
}