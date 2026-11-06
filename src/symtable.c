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