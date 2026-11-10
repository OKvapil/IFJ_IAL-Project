/**
 * @file ast.c
 * @brief Implementation of abstract syntax tree functions
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

AstNode *ast_if(AstNode *cond, AstNode *then_block, AstNode *else_block,
                size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_IF;
  node->line = line;
  node->var_id = 0;
  node->data.if_state.cond = cond;
  node->data.if_state.then_block = then_block;
  node->data.if_state.else_block = else_block;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_while(AstNode *cond, AstNode *block, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_WHILE;
  node->line = line;
  node->var_id = 0;
  node->data.while_state.cond = cond;
  node->data.while_state.block = block;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_expr_assign(char *name, AstNode *value, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_ASSIGN;
  node->line = line;
  node->var_id = 0;
  node->data.expr_assign.name = name;
  node->data.expr_assign.value = value;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_binary(TokenType op, AstNode *left, AstNode *right, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_BINARY;
  node->data.binary.op = op;
  node->line = line;
  node->var_id = 0;
  node->data.binary.left = left;
  node->data.binary.right = right;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_unary(TokenType op, AstNode *operand, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_UNARY;
  node->line = line;
  node->var_id = 0;
  node->data.unary.op = op;
  node->data.unary.operand = operand;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_from_token(Token *value) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->line = value->line;
  node->var_id = 0;
  switch (value->type) {
  case TOKEN_INT_LIT:
    node->kind = AST_INT_LIT;
    node->data.int_value = value->attr.int_val;
    node->result_type = TYPE_INT;
    break;
  case TOKEN_DOUBLE_LIT:
    node->kind = AST_DOUBLE_LIT;
    node->data.float_value = value->attr.float_val;
    node->result_type = TYPE_DOUBLE;
    break;
  case TOKEN_STRING_LIT:
    node->kind = AST_STRING_LIT;
    node->data.text = value->attr.str_val;
    value->attr.str_val = NULL;
    node->result_type = TYPE_STRING;
    break;
  case TOKEN_IDENTIFIER:
    node->kind = AST_IDENTIFIER;
    node->data.text = value->attr.str_val;
    value->attr.str_val = NULL;
    node->result_type = TYPE_UNKNOWN;
    break;
  default:
    free(node);
    return NULL;
  }
  return node;
}

AstNode *ast_block(void) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_BLOCK;
  node->line = 0;
  node->var_id = 0;
  node->data.block.capacity = 0;
  node->data.block.count = 0;
  node->data.block.statements = NULL;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_block_add_stmt(AstNode *block, AstNode *statement) {
  if (block == NULL || statement == NULL) {
    return NULL;
  }
  if (block->data.block.count >= block->data.block.capacity) {
    size_t new_cap =
        (block->data.block.capacity == 0) ? 2 : block->data.block.capacity * 2;
    AstNode **new_stmts =
        realloc(block->data.block.statements, new_cap * sizeof(AstNode *));
    if (new_stmts == NULL) {
      return NULL;
    }
    block->data.block.statements = new_stmts;
    block->data.block.capacity = new_cap;
  }

  block->data.block.statements[block->data.block.count] = statement;
  block->data.block.count++;
  return block;
}

AstNode *ast_var_decl(AstKind kind, char *name, DataType type,
                      AstNode *init_expr, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }

  node->kind = kind;
  node->line = line;
  node->var_id = 0;
  node->data.var_dec.name = name;
  node->data.var_dec.init_expr = init_expr;
  node->data.var_dec.var_type = type;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_func_def(char *name, DataType return_type, AstNode *params,
                      size_t param_count, AstNode *block, bool is_main,
                      size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_FUNC_DEF;
  node->line = line;
  node->var_id = 0;
  node->data.func_def.name = name;
  node->data.func_def.params = params;
  node->data.func_def.param_count = param_count;
  node->data.func_def.block = block;
  node->data.func_def.is_main = is_main;
  node->data.func_def.return_type = return_type;
  node->result_type = return_type;
  return node;
}

AstNode *ast_func_call(char *name, AstNode **args, size_t arg_count,
                       size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_FUNC_CALL;
  node->line = line;
  node->var_id = 0;
  node->data.func_call.name = name;
  node->data.func_call.arg_count = arg_count;
  node->data.func_call.args = args;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_free(AstNode *node) {
  if (node == NULL) {
    return NULL;
  }

  switch (node->kind) {
  case AST_UNARY:
    ast_free(node->data.unary.operand);
    break;
  case AST_BINARY:
    ast_free(node->data.binary.left);
    ast_free(node->data.binary.right);
    break;
  case AST_IF:
    ast_free(node->data.if_state.else_block);
    ast_free(node->data.if_state.then_block);
    ast_free(node->data.if_state.cond);
    break;
  case AST_WHILE:
    ast_free(node->data.while_state.block);
    ast_free(node->data.while_state.cond);
    break;
  case AST_ASSIGN:
    free(node->data.expr_assign.name);
    ast_free(node->data.expr_assign.value);
    break;
  case AST_FUNC_CALL:
    free(node->data.func_call.name);
    for (size_t i = 0; i < node->data.func_call.arg_count; i++) {
      ast_free(node->data.func_call.args[i]);
    }
    free(node->data.func_call.args);
    break;
  case AST_BLOCK:
    for (size_t i = 0; i < node->data.block.count; i++) {
      ast_free(node->data.block.statements[i]);
    }
    free(node->data.block.statements);
    break;
  case AST_FUNC_DEF:
    ast_free(node->data.func_def.block);
    free(node->data.func_def.name);
    ast_free(node->data.func_def.params);
    break;
  case AST_STRING_LIT:
    free(node->data.text);
    break;
  case AST_IDENTIFIER:
    free(node->data.text);
    break;
  case AST_VAL:
    ast_free(node->data.var_dec.init_expr);
    free(node->data.var_dec.name);
    break;
  case AST_VAR:
    ast_free(node->data.var_dec.init_expr);
    free(node->data.var_dec.name);
    break;
  case AST_RETURN:
    ast_free(node->data.return_state.expression);
    break;
  default:
    break;
  }
  free(node);
  return NULL;
}

AstNode *ast_return(AstNode *expr, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_RETURN;
  node->line = line;
  node->var_id = 0;
  node->data.return_state.expression = expr;
  node->result_type = TYPE_UNKNOWN;
  return node;
}