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

static bool is_numeric(DataType type) {
  if (type == TYPE_INT || type == TYPE_DOUBLE) {
    return true;
  }
  return false;
}

static DataType binary_result_type(DataType op1_type, DataType op2_type) {
  if (!is_numeric(op1_type) || !is_numeric(op2_type)) {
    return TYPE_UNKNOWN;
  } else if (op1_type == TYPE_DOUBLE || op2_type == TYPE_DOUBLE) {
    return TYPE_DOUBLE;
  } else {
    return TYPE_INT;
  }
}

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

  case AST_IDENTIFIER: {
    Symbol *symbol = st_lookup(symtable, node->data.text);
    if (symbol == NULL) {
      return ERR_UNDEFINED;
    }
    node->result_type = symbol->type;
    node->var_id = symbol->var_unique_id;
    break;
  }

  case AST_UNARY: {
    TokenType operation = node->data.unary.op;
    AstNode *operand = node->data.unary.operand;

    ErrorCode analyze_operand = analyze_node(operand, symtable);
    if (analyze_operand != ERR_OK) {
      return analyze_operand;
    }

    if (operation == TOKEN_MINUS || operation == TOKEN_PLUS) {
      if (operand->result_type != TYPE_INT &&
          operand->result_type != TYPE_DOUBLE) {
        return ERR_TYPE;
      }
      node->result_type = node->data.unary.operand->result_type;
    }

    else if (operation == TOKEN_NOT) {
      if (operand->result_type != TYPE_BOOL) {
        return ERR_TYPE;
      }
      node->result_type = TYPE_BOOL;
    }

    else {
      return ERR_INTERNAL;
    }
    break;
  }

  case AST_BINARY: {
    TokenType operation = node->data.binary.op;
    AstNode *operand1 = node->data.binary.left;
    AstNode *operand2 = node->data.binary.right;

    ErrorCode analyze_operand1 = analyze_node(operand1, symtable);
    if (analyze_operand1 != ERR_OK) {
      return analyze_operand1;
    }

    ErrorCode analyze_operand2 = analyze_node(operand2, symtable);
    if (analyze_operand2 != ERR_OK) {
      return analyze_operand2;
    }

    /* AI-generated (Claude). Prompt: "rewrite this chain of ifs into the switch
    case" (binary arithmetic operators). */
    switch (operation) {
    case TOKEN_PLUS:
      if (operand1->result_type == TYPE_STRING &&
          operand2->result_type == TYPE_STRING) {
        node->result_type = TYPE_STRING;
        break;
      }

    /* fall through */
    case TOKEN_MINUS:
    case TOKEN_MUL:
    case TOKEN_DIV: {
      DataType result_type =
          binary_result_type(operand1->result_type, operand2->result_type);
      if (result_type == TYPE_UNKNOWN) {
        return ERR_TYPE;
      }
      node->result_type = result_type;
      break;
    }

    case TOKEN_LT:
    case TOKEN_LTE:
    case TOKEN_GT:
    case TOKEN_GTE: {
      if ((is_numeric(operand1->result_type) &&
           is_numeric(operand2->result_type)) ||
          (operand1->result_type == TYPE_STRING &&
           operand2->result_type == TYPE_STRING)) {
        node->result_type = TYPE_BOOL;
        break;
      } else {
        return ERR_TYPE;
      }
    }

    case TOKEN_EQ:
    case TOKEN_NEQ: {
      if (operand1->result_type == TYPE_UNKNOWN ||
          operand2->result_type == TYPE_UNKNOWN) {
        return ERR_TYPE;
      }
      if ((is_numeric(operand1->result_type) &&
           is_numeric(operand2->result_type)) ||
          operand1->result_type == operand2->result_type) {
        node->result_type = TYPE_BOOL;
        break;
      } else {
        return ERR_TYPE;
      }
    }

    case TOKEN_AND:
    case TOKEN_OR: {
      if (operand1->result_type != TYPE_BOOL ||
          operand2->result_type != TYPE_BOOL) {
        return ERR_TYPE;
      }
      node->result_type = TYPE_BOOL;
      break;
    }

    default:
      return ERR_INTERNAL;
    }

    break;
  }

  case AST_FUNC_DEF:
    break;

  case AST_FUNC_CALL:
    break;

  case AST_RETURN:
    break;

  case AST_IF:
    break;

  case AST_WHILE:
    break;

  case AST_BLOCK:
  {
    if (st_enter_scope(symtable) == -1) {
      return ERR_INTERNAL;
    }

    for (size_t i = 0; i < node->data.block.count; i++) {
      ErrorCode node_result_error = analyze_node(node->data.block.statements[i], symtable);
      if (node_result_error != ERR_OK) {
        return node_result_error;
      }
    }
    DataType block_type = TYPE_UNIT;
    if (node->data.block.count > 0) {
      block_type = node->data.block.statements[node->data.block.count - 1]->result_type;
    }
    node->result_type = block_type;
    st_exit_scope(symtable);
    break;
  }

  case AST_ASSIGN:
    break;

  case AST_VAR:
  case AST_VAL:
  {
    if (node->data.var_dec.init_expr == NULL) {
      return ERR_INTERNAL;
    }

    ErrorCode init_result = analyze_node(node->data.var_dec.init_expr, symtable);
    if (init_result != ERR_OK) {
      return init_result;
    }

    if (node->data.var_dec.var_type != TYPE_UNKNOWN) {
      if (node->data.var_dec.init_expr->result_type != node->data.var_dec.var_type) {
        return ERR_TYPE;
      }
    } else {
      if (node->data.var_dec.init_expr->result_type == TYPE_UNKNOWN) {
        return ERR_TYPE;
      } else {
        node->data.var_dec.var_type = node->data.var_dec.init_expr->result_type;
      }
    }

    Symbol *symbol = NULL;
    DeclarationCode decl_result = st_declare_var(symtable, node->data.var_dec.name, node->data.var_dec.var_type, node->kind == AST_VAR, &symbol);
    if (decl_result == DECL_REDEF) {
      return ERR_REDEFINITION;
    } else if (decl_result == DECL_ALLOC_FAIL) {
      return ERR_INTERNAL;
    }

    node->var_id = symbol->var_unique_id;
    
    node->result_type = TYPE_UNIT;
 
    break;
  }
    
  }

  return ERR_OK;
}