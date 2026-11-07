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
  node->data.unary.op = op;
  node->line = line;
  node->var_id = 0;
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

AstNode *ast_return(AstNode *expr, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }

  node->data.return_state.expression = expr;
  node->var_id = 0;
  node->line = line;
  node->kind = AST_RETURN;
  node->result_type = TYPE_UNKNOWN;
  return node;
}

AstNode *ast_block_add_stmt(AstNode *block, AstNode *statement) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }

  node->line = statement->line;
  node->var_id = 0;
  return node;
}

AstNode *ast_block(AstNode *block) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->line = 0;
  node->var_id = 0;
  node->result_type = TYPE_UNKNOWN;

  node->data.block.capacity = 0;
  node->data.block.count = 0;
  node->data.block.statements = NULL;
  return node;
}

AstNode *ast_var_decl(AstKind kind, char *name, DataType type,
                      AstNode *init_expr, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }

  node->kind = kind;
  node->var_id = 0;
  node->line = line;
  node->data.var_dec.name = name;
  node->result_type = type;
  node->data.var_dec.init_expr = init_expr;
  return node;
}

AstNode *ast_func_call(char *name, AstNode **args, size_t arg_count,
                       size_t line) {}

AstNode ast_func_def() {}

AstNode *ast_free(AstNode *node) {
  if (node == NULL) {
    return NULL;
  }

  switch (node->kind) {
  case AST_UNARY:
    free(node->data.unary.operand);
  case AST_BINARY:
    free(node->data.binary.left);
    free(node->data.binary.right);
  case AST_IF:
    free(node->data.if_state.else_block);
    free(node->data.if_state.then_block);
  case AST_WHILE:
    free(node->data.while_state.block);
    free(node->data.while_state.cond);
  case AST_ASSIGN:
    free(node->data.expr_assign.name);
    free(node->data.expr_assign.value);
    break;
  case AST_FUNC_CALL:
    free(node->data.func_call.args);
    free(node->data.func_call.name);
  case AST_BLOCK:
    free(node->data.block.statements);
    case AST_FUNC_DEF:
    free(node->data.func_def.block);
    free(node->data.func_def.name);
    free(node->data.func_def.params);
  }

  free(node);
  return NULL;
}
