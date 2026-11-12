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

static DataType current_return_type = TYPE_UNKNOWN;

static bool needs_int2float(DataType type1, DataType type2) {
  if (type1 == TYPE_INT && type2 == TYPE_DOUBLE) {
    return true;
  }
  return false;
}

static bool is_expr_node(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  switch (node->kind) {
  case AST_INT_LIT:
  case AST_DOUBLE_LIT:
  case AST_STRING_LIT:
  case AST_IDENTIFIER:
  case AST_BINARY:
  case AST_UNARY:
  case AST_FUNC_CALL:
    return true;
  default:
    return false;
  }
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

static bool var_name(char *buff, size_t size, const char *name, int var_id) {
  if (name == NULL || var_id < 0) {
    return false;
  }

  int len = snprintf(buff, size, "LF@%s$%d", name, var_id);
  if (len < 0 || (size_t)len >= size) {
    return false;
  }
  return true;
}

static bool flow_label(char *buff, size_t size, int id, const char *kind) {
  if (kind == NULL) {
    return false;
  }

  int len = snprintf(buff, size, "$L%d_%s", id, kind);
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

static bool gen_assign(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  char name[256];
  if (!var_name(name, sizeof(name), node->data.expr_assign.name,
                node->var_id)) {
    return false;
  }

  if (!gen_expr(node->data.expr_assign.value)) {
    return false;
  }

  if (needs_int2float(node->data.expr_assign.value->result_type,
                      node->result_type)) {
    codegen_int2floats();
  }

  codegen_pops(name);
  return true;
}

static bool gen_block(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  for (size_t i = 0; i < node->data.block.count; i++) {
    if (!gen_statement(node->data.block.statements[i])) {
      return false;
    }
  }
  return true;
}

static bool gen_var_decl(const AstNode *node) {
  if (node == NULL || (node->kind != AST_VAL && node->kind != AST_VAR)) {
    return false;
  }

  const char *name = node->data.var_dec.name;
  const AstNode *value = node->data.var_dec.init_expr;
  if (name == NULL || value == NULL) {
    return false;
  }

  if (!gen_expr(value)) {
    return false;
  }

  if (node->data.var_dec.var_type == TYPE_DOUBLE &&
      value->result_type == TYPE_INT) {
    codegen_int2floats();
  }

  char var[256];
  if (!var_name(var, sizeof(var), name, node->var_id)) {
    return false;
  }
  codegen_pops(var);
  return true;
}

static bool gen_def_vars(const AstNode *node) {
  if (node == NULL) {
    return true;
  }

  switch (node->kind) {
  case AST_VAR:
  case AST_VAL: {
    char name[256];
    if (!var_name(name, sizeof(name), node->data.var_dec.name, node->var_id)) {
      return false;
    }
    codegen_defvar(name);
    return true;
  }

  case AST_BLOCK:
    for (size_t i = 0; i < node->data.block.count; i++) {
      if (!gen_def_vars(node->data.block.statements[i])) {
        return false;
      }
    }
    return true;

  case AST_IF:
    if (!gen_def_vars(node->data.if_state.then_block)) {
      return false;
    }
    if (!gen_def_vars(node->data.if_state.else_block)) {
      return false;
    }
    return true;

  case AST_WHILE:
    return gen_def_vars(node->data.while_state.block);

  default:
    return true;
  }
}

static bool gen_return_value(const AstNode *expr) {
  if (expr == NULL) {
    codegen_pushs("nil@nil");
  } else {
    if (!gen_expr(expr)) {
      return false;
    }
    if (current_return_type == TYPE_UNIT) {
      codegen_pops("GF@$trash");
      codegen_pushs("nil@nil");
    } else if (current_return_type == TYPE_DOUBLE &&
               expr->result_type == TYPE_INT) {
      codegen_int2floats();
    }
  }
  codegen_popframe();
  codegen_return();
  return true;
}

static bool gen_function(const AstNode *node) {
  if (node == NULL || node->kind != AST_FUNC_DEF) {
    return false;
  }

  const char *name = node->data.func_def.name;
  AstNode **params = node->data.func_def.params;
  size_t param_count = node->data.func_def.param_count;
  const AstNode *body = node->data.func_def.block;

  if (name == NULL || body == NULL) {
    return false;
  }
  if (params == NULL && param_count > 0) {
    return false;
  }

  current_return_type = node->data.func_def.return_type;

  char label[256];
  if (!fn_label(label, sizeof(label), name)) {
    return false;
  }
  codegen_label(label);
  codegen_createframe();
  codegen_pushframe();

  for (size_t i = 0; i < param_count; i++) {
    if (params[i] == NULL || !gen_def_vars(params[i])) {
      return false;
    }
  }

  if (!gen_def_vars(body)) {
    return false;
  }

  char var[256];
  for (size_t i = param_count; i > 0; i--) {
    if (!var_name(var, sizeof(var), params[i - 1]->data.var_dec.name,
                  params[i - 1]->var_id)) {
      return false;
    }
    codegen_pops(var);
  }

  if (body->kind != AST_BLOCK) {
    return gen_return_value(body);
  }

  size_t count = body->data.block.count;
  AstNode **statements = body->data.block.statements;
  for (size_t i = 0; i + 1 < count; i++) {
    if (!gen_statement(statements[i])) {
      return false;
    }
  }

  if (count > 0 && is_expr_node(statements[count - 1])) {
    return gen_return_value(statements[count - 1]);
  }

  if (count > 0 && !gen_statement(statements[count - 1])) {
    return false;
  }
  codegen_pushs("nil@nil");
  codegen_popframe();
  codegen_return();
  return true;
}

static bool gen_return(const AstNode *node) {
  if (node == NULL || node->kind != AST_RETURN) {
    return false;
  }
  return gen_return_value(node->data.return_state.expression);
}

static bool gen_call(const AstNode *node) {
  if (node == NULL || node->kind != AST_FUNC_CALL) {
    return false;
  }

  const char *name = node->data.func_call.name;
  AstNode **args = node->data.func_call.args;
  size_t arg_count = node->data.func_call.arg_count;

  if (name == NULL) {
    return false;
  }

  if (args == NULL && arg_count > 0) {
    return false;
  }

  for (size_t i = 0; i < arg_count; i++) {
    if (args[i] == NULL) {
      return false;
    }
    if (!gen_expr(args[i])) {
      return false;
    }
  }

  char label[256];
  if (!fn_label(label, sizeof(label), name)) {
    return false;
  }
  codegen_call(label);

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
    char name[256];
    if (!var_name(name, sizeof(name), node->data.text, node->var_id)) {
      return false;
    }
    codegen_pushs(name);
    return true;
  }
  case AST_BINARY:
    return gen_binary(node);
  case AST_UNARY:
    return gen_unary(node);
  case AST_FUNC_CALL:
    return gen_call(node);
  default:
    return false;
  }
}

static bool gen_while(const AstNode *node) {
  if (node == NULL) {
    return false;
  }
  char name_start[256];
  char name_end[256];
  int id = codegen_new_label_id();

  if (!flow_label(name_start, sizeof(name_start), id, "while")) {
    return false;
  }
  if (!flow_label(name_end, sizeof(name_end), id, "while_end")) {
    return false;
  }

  codegen_label(name_start);

  if (!gen_expr(node->data.while_state.cond)) {
    return false;
  }
  codegen_pushs("bool@false");
  codegen_jumpifeqs(name_end);

  if (!gen_statement(node->data.while_state.block)) {
    return false;
  }
  codegen_jump(name_start);

  codegen_label(name_end);

  return true;
}

static bool gen_if(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  char name_else[256];
  char name_end[256];
  int id = codegen_new_label_id();

  if (!flow_label(name_else, sizeof(name_else), id, "else")) {
    return false;
  }
  if (!flow_label(name_end, sizeof(name_end), id, "if_end")) {
    return false;
  }

  if (!gen_expr(node->data.if_state.cond)) {
    return false;
  }
  codegen_pushs("bool@false");

  if (node->data.if_state.else_block != NULL) {
    codegen_jumpifeqs(name_else);
  } else {
    codegen_jumpifeqs(name_end);
  }

  if (!gen_statement(node->data.if_state.then_block)) {
    return false;
  }

  if (node->data.if_state.else_block != NULL) {
    codegen_jump(name_end);
    codegen_label(name_else);
    if (!gen_statement(node->data.if_state.else_block)) {
      return false;
    }
  }

  codegen_label(name_end);

  return true;
}

bool gen_statement(const AstNode *node) {
  if (node == NULL) {
    return false;
  }

  switch (node->kind) {
  case AST_ASSIGN:
    return gen_assign(node);
  case AST_WHILE:
    return gen_while(node);
  case AST_IF:
    return gen_if(node);
  case AST_BLOCK:
    return gen_block(node);
  case AST_VAL:
  case AST_VAR:
    return gen_var_decl(node);
  case AST_RETURN:
    return gen_return(node);
  default:
    if (is_expr_node(node)) {
      if (!gen_expr(node)) {
        return false;
      }
      codegen_pops("GF@$trash");
      return true;
    }
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

bool gen_program(const AstNode *root) {
  if (root == NULL || root->kind != AST_BLOCK) {
    return false;
  }

  const char *main_name = NULL;
  for (size_t i = 0; i < root->data.block.count; i++) {
    const AstNode *func = root->data.block.statements[i];
    if (func == NULL || func->kind != AST_FUNC_DEF) {
      return false;
    }
    if (func->data.func_def.is_main) {
      main_name = func->data.func_def.name;
    }
  }
  if (main_name == NULL) {
    return false;
  }

  if (!gen_program_start(main_name)) {
    return false;
  }

  for (size_t i = 0; i < root->data.block.count; i++) {
    if (!gen_function(root->data.block.statements[i])) {
      return false;
    }
  }

  gen_program_end();
  return true;
}
