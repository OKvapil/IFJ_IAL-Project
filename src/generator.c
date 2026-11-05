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

static bool gen_binary(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  if (!gen_expr(node->data.binary.left)) {
    return false;
  }
  if (!gen_expr(node->data.binary.right)) {
    return false;
  }

  switch (node->data.binary.op) {
  case TOKEN_PLUS:
    codegen_adds();
    break;
  case TOKEN_MINUS:
    codegen_subs();
    break;
  case TOKEN_MUL:
    codegen_muls();
    break;
  case TOKEN_LT:
    codegen_lts();
    break;
  case TOKEN_GT:
    codegen_gts();
    break;
  case TOKEN_EQ:
    codegen_eqs();
    break;
  case TOKEN_NEQ:
    codegen_eqs();
    codegen_nots();
    break;
  case TOKEN_LTE:
    codegen_gts();
    codegen_nots();
    break;
  case TOKEN_GTE:
    codegen_lts();
    codegen_nots();
    break;
  default:
    return false;
  }
  return true;
}

static bool gen_unary(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  if (!gen_expr(node->data.unary.operand)) {
    return false;
  }

  switch (node->data.unary.op) {
  case TOKEN_PLUS:
    break;
  case TOKEN_MINUS:
    if (node->result_type == TYPE_INT) {
      codegen_pushs("int@-1");
    } else if (node->result_type == TYPE_DOUBLE) {
      codegen_pushs("float@-0x1p+0");
    } else {
      return false;
    }
    codegen_muls();
    break;
  default:
    return false;
  }
  return true;
}

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
    if (node->data.text == NULL) {
      return false;
    }

    char name[256];

    int len =
        snprintf(name, sizeof(name), "LF@%s$%d", node->data.text, node->var_id);
    if (len < 0 || (size_t)len >= sizeof(name)) {
      return false;
    }

    codegen_pushs(name);
    return true;
  }
  case AST_BINARY:
    return gen_binary(node);
  case AST_UNARY:
    return gen_unary(node);
  default:
    return false;
  }
}