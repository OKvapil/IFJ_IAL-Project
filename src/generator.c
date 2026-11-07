/**
 * @file generator.c
 * @brief Generates IFJcode26 from the abstract syntax tree (implementation)
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

static bool needs_int2float(DataType type1, DataType type2) {
  if (type1 == TYPE_INT && type2 == TYPE_DOUBLE) {
    return true;
  }
  return false;
}

// Function label creation
static bool fn_label(char *buff, size_t size, const char *name) {
  if (name == NULL) {
    return false;
  }

  int len = snprintf(buff, size, "$fn_%s", name);
  if (len < 0 || (size_t)len >= size) {
    return false;
  }
  return true;
}

static bool gen_binary(const AstNode *node) {
  if (node == NULL || node->data.binary.left == NULL ||
      node->data.binary.right == NULL) {
    return false;
  }

  bool int_div =
      node->data.binary.op == TOKEN_DIV && node->result_type == TYPE_INT;

  if (!gen_expr(node->data.binary.left)) {
    return false;
  }
  if (int_div || needs_int2float(node->data.binary.left->result_type,
                                 node->data.binary.right->result_type)) {
    codegen_int2floats();
  }

  if (!gen_expr(node->data.binary.right)) {
    return false;
  }
  if (int_div || needs_int2float(node->data.binary.right->result_type,
                                 node->data.binary.left->result_type)) {
    codegen_int2floats();
  }

  switch (node->data.binary.op) {
  case TOKEN_PLUS:
    if (node->result_type == TYPE_STRING) {
      codegen_pops("GF@$concat_r");
      codegen_pops("GF@$concat_l");
      codegen_concat("GF@$concat_l", "GF@$concat_l", "GF@$concat_r");
      codegen_pushs("GF@$concat_l");
    } else {
      codegen_adds();
    }
    break;
  case TOKEN_MINUS:
    codegen_subs();
    break;
  case TOKEN_MUL:
    codegen_muls();
    break;
  case TOKEN_DIV:
    codegen_divs();
    if (int_div) {
      codegen_float2ints();
    }
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

bool gen_program_start(const char *main_name) {
  if (main_name == NULL) {
    return false;
  }

  char label[256];
  if (!fn_label(label, sizeof(label), main_name)) {
    return false;
  }

  codegen_header();
  codegen_defvar("GF@$trash");
  codegen_defvar("GF@$concat_l");
  codegen_defvar("GF@$concat_r");
  codegen_call(label);
  codegen_pops("GF@$trash");
  codegen_exit("int@0");
  return true;
}

// Build-in functions will be at the end of the program (they are written in ifj
// code)
void gen_program_end(void) {
  builtin_substr_definition();
  builtin_ord_definition();
}
