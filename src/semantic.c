/**
 * @file semantic.c
 * @brief The implementation of the semantic analysis.
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Jonáš Ľalík (xlalikj00)
 */

#include "semantic.h"
#include "symtable.h"

static ErrorCode analyze_node(AstNode *node, SymTable *symtable) {
  switch (node->kind) {
  case AST_INT_LIT:
    node->result_type = TYPE_INT;
    break;

  case AST_DOUBLE_LIT:
    node->result_type = TYPE_DOUBLE;
    break;

  case AST_STRING_LIT:
    node->result_type = TYPE_STRING;
    break;

  case AST_IDENTIFIER:
    Symbol *symbol = st_lookup(symtable, node->data.text);
    if (symbol == NULL) {
      return ERR_UNDEFINED;
    }
    node->result_type = symbol->type;
    node->var_id = symbol->var_unique_id;
    break;

  case AST_UNARY:
    break;

  case AST_BINARY:
    break;

  case AST_FUNC_DEF:
    break;

    /* case AST_FUNC_CALL:  commented out because of unavailible kinds
      break;

    case AST_RETURN:
      break; */

  case AST_IF:
    break;

  case AST_WHILE:
    break;

  case AST_BLOCK:
    break;

  case AST_ASSIGN:
    break;

  case AST_VAL:
    break;

  case AST_VAR:
    break;
  }

  return ERR_OK;
}