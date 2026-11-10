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
#include <string.h>

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
    {"print", TYPE_UNIT, 1, {TYPE_ANY}},
    {"readLine", TYPE_STRING, 0, {0}},
    {"readInt", TYPE_INT, 0, {0}},
    {"readDouble", TYPE_DOUBLE, 0, {0}},
    {"readBoolean", TYPE_BOOL, 0, {0}},
    {"toStr", TYPE_STRING, 1, {TYPE_ANY}},
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

int st_enter_scope(SymTable *symtable) {
  SymScope *new_scope = malloc(sizeof(SymScope));
  if (new_scope == NULL) {
    return -1;
  }

  HashTable *hash_table = ht_init();
  if (hash_table == NULL) {
    free(new_scope);
    return -1;
  }
  new_scope->table = hash_table;

  new_scope->outer = symtable->current_scope;
  symtable->current_scope = new_scope;

  return 0;
}

int st_exit_scope(SymTable *symtable) {
  if (symtable->current_scope->outer == NULL) {
    return -1;
  }

  pop_scope(symtable);
  return 0;
}

DeclarationCode st_declare_var(SymTable *symtable, const char *name,
                               DataType type, bool modifiable,
                               Symbol **return_symbol) {

  if (ht_find(symtable->current_scope->table, name) != NULL) {
    return DECL_REDEF;
  }

  Symbol *new_symbol = calloc(1, sizeof(Symbol));
  if (new_symbol == NULL) {
    return DECL_ALLOC_FAIL;
  }

  new_symbol->name = malloc(strlen(name) + 1);
  if (new_symbol->name == NULL) {
    free(new_symbol);
    return DECL_ALLOC_FAIL;
  }
  strcpy(new_symbol->name, name);

  new_symbol->kind = SYMBOL_VARIABLE;
  new_symbol->type = type;
  new_symbol->var_modifiable = modifiable;
  new_symbol->var_unique_id = symtable->var_counter;

  if (ht_add(symtable->current_scope->table, name, new_symbol) == NULL) {
    free_symbol(new_symbol);
    return DECL_ALLOC_FAIL;
  }

  if (return_symbol != NULL) {
    *return_symbol = new_symbol;
  }

  symtable->var_counter += 1;
  return DECL_OK;
}

DeclarationCode st_declare_fun(SymTable *symtable, const char *name,
                               DataType type, int param_count,
                               const DataType *param_types,
                               const char *const *param_names,
                               Symbol **return_symbol) {

  SymScope *global_scope = symtable->current_scope;
  while (global_scope->outer != NULL) {
    global_scope = global_scope->outer;
  }

  if (ht_find(global_scope->table, name) != NULL) {
    return DECL_REDEF;
  }

  Symbol *new_symbol = calloc(1, sizeof(Symbol));
  if (new_symbol == NULL) {
    return DECL_ALLOC_FAIL;
  }

  new_symbol->name = malloc(strlen(name) + 1);
  if (new_symbol->name == NULL) {
    free_symbol(new_symbol);
    return DECL_ALLOC_FAIL;
  }
  strcpy(new_symbol->name, name);

  new_symbol->param_count = param_count;
  if (param_count > 0) {
    new_symbol->param_types = malloc(param_count * sizeof(DataType));
    if (new_symbol->param_types == NULL) {
      free_symbol(new_symbol);
      return DECL_ALLOC_FAIL;
    }

    memcpy(new_symbol->param_types, param_types,
           param_count * sizeof(DataType));
  }

  if (param_names != NULL && param_count > 0) {
    new_symbol->param_names = calloc(param_count, sizeof(char *));
    if (new_symbol->param_names == NULL) {
      free_symbol(new_symbol);
      return DECL_ALLOC_FAIL;
    }

    for (int i = 0; i < param_count; i++) {
      new_symbol->param_names[i] = malloc(strlen(param_names[i]) + 1);
      if (new_symbol->param_names[i] == NULL) {
        free_symbol(new_symbol);
        return DECL_ALLOC_FAIL;
      }
      strcpy(new_symbol->param_names[i], param_names[i]);
    }
  }

  new_symbol->kind = SYMBOL_FUNCTION;
  new_symbol->type = type;

  if (ht_add(global_scope->table, name, new_symbol) == NULL) {
    free_symbol(new_symbol);
    return DECL_ALLOC_FAIL;
  }

  if (return_symbol != NULL) {
    *return_symbol = new_symbol;
  }

  return DECL_OK;
}

Symbol *st_lookup(const SymTable *symtable, const char *name) {
  SymScope *tmp = symtable->current_scope;

  while (tmp != NULL) {
    Entry *ht_entry = ht_find(tmp->table, name);
    if (ht_entry != NULL) {
      return (Symbol *)ht_entry->data;
    }

    tmp = tmp->outer;
  }

  return NULL;
}