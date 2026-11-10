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

static AstNode *ast_new(AstKind kind, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    exit(99);
  }
  node->kind = kind;
  node->line = line;
  node->var_id = 0;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_if(AstNode *cond, AstNode *then_block, AstNode *else_block,
                size_t line) {

  AstNode *node = ast_new(AST_IF, line);
  node->data.if_state.cond = cond;
  node->data.if_state.then_block = then_block;
  node->data.if_state.else_block = else_block;
  return node;
}

AstNode *ast_while(AstNode *cond, AstNode *block, size_t line) {

  AstNode *node = ast_new(AST_WHILE, line);
  node->data.while_state.cond = cond;
  node->data.while_state.block = block;
  return node;
}

AstNode *ast_expr_assign(char *name, AstNode *value, size_t line) {

  AstNode *node = ast_new(AST_ASSIGN, line);
  node->data.expr_assign.name = name;
  node->data.expr_assign.value = value;
  return node;
}

AstNode *ast_binary(TokenType op, AstNode *left, AstNode *right, size_t line) {

  AstNode *node = ast_new(AST_BINARY, line);
  node->data.binary.op = op;
  node->data.binary.left = left;
  node->data.binary.right = right;
  return node;
}

AstNode *ast_unary(TokenType op, AstNode *operand, size_t line) {

  AstNode *node = ast_new(AST_UNARY, line);
  node->data.unary.op = op;
  node->data.unary.operand = operand;
  return node;
}

AstNode *ast_from_token(Token *value) {

  AstNode *node = ast_new(AST_INT_LIT, value->line);
  if (value == NULL) {
    return NULL;
  }
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
  AstNode *node = ast_new(AST_BLOCK, 0);
  node->data.block.capacity = 0;
  node->data.block.count = 0;
  node->data.block.statements = NULL;
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
      exit(99);
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
  AstNode *node = ast_new(kind, line);
  node->data.var_dec.name = name;
  node->data.var_dec.init_expr = init_expr;
  node->data.var_dec.var_type = type;
  return node;
}

AstNode *ast_func_def(char *name, DataType return_type, AstNode **params,
                      size_t param_count, AstNode *block, bool is_main,
                      size_t line) {
  AstNode *node = ast_new(AST_FUNC_DEF, line);
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
  AstNode *node = ast_new(AST_FUNC_CALL, line);
  node->data.func_call.name = name;
  node->data.func_call.arg_count = arg_count;
  node->data.func_call.args = args;
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
    for (size_t i = 0; i < node->data.func_def.param_count; i++) {
      ast_free(node->data.func_def.params[i]);
    }
    free(node->data.func_def.params);
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
  AstNode *node = ast_new(AST_RETURN, line);
  node->data.return_state.expression = expr;
  return node;
}