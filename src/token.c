/**
 * @file token.c
 * @brief Token utility functions implementation for the IFJ26 compiler
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Filip Haza (xhazafi00)
 */

#include "token.h"
#include <stdlib.h>

void token_clear(Token *tok) {
  if (tok == NULL) {
    return;
  }
  if (tok->type == TOKEN_IDENTIFIER || tok->type == TOKEN_STRING_LIT) {
    if (tok->attr.str_val != NULL) {
      free(tok->attr.str_val);
      tok->attr.str_val = NULL;
    }
  }
}