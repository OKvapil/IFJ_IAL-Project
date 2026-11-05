/**
 * @file generator.c
 * @brief Generates IFJcode26 from the abstract syntax tree (interface)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#include "generator.h"
#include "builtins.h"
#include "codegen.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool gen_expr(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  switch (node->kind) {
  case AST_INT_LIT: {
    char *op = codegen_const_int(node->data.int_value);
    if (op == NULL) {
      return false;
    }
    codegen_pushs(op);
    free(op);
    return true;
  }
  case AST_DOUBLE_LIT: {
    char *op = codegen_const_float(node->data.float_value);
    if (op == NULL) {
      return false;
    }
    codegen_pushs(op);
    free(op);
    return true;
  }
  case AST_STRING_LIT: {
    char *op = codegen_const_string(node->data.text);
    if (op == NULL) {
      return false;
    }
    codegen_pushs(op);
    free(op);
    return true;
  }
  case AST_IDENTIFIER: {
    
    return true;
  }
  default:
    return false;
  }
}