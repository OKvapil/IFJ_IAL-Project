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

AstNode *ast_new_if(AstNode *cond, AstNode *then_block, AstNode *else_block,
                    size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_IF;
  node->line = line;
  node->data.if_state.cond = cond;
  node->data.if_state.then_block = then_block;
  node->data.if_state.else_block = else_block;
  return node;
}

AstNode *ast_new_while(AstNode *cond, AstNode *block, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_WHILE;
  node->line = line;
  node->data.while_state.cond = cond;
  node->data.while_state.block = block;
  return node;
}

AstNode *ast_new_expr_assign(char *name, AstNode *value, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_ASSIGN;
  node->line = line;
  node->data.expr_assign.name = name;
  node->data.expr_assign.value = value;
  return node;
}

AstNode *ast_new_binary(TokenType op, AstNode *left, AstNode *right,
                        size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_BINARY;
  node->data.binary.op = op;
  node->line = line;
  node->data.binary.left = left;
  node->data.binary.right = right;
  return node;
}

AstNode *ast_new_unary(TokenType op, AstNode *operand, size_t line) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  node->kind = AST_UNARY;
  node->data.unary.op = op;
  node->line = line;
  node->data.unary.operand = operand;
  return node;
}

AstNode *ast_new_from_token(Token *value) {
  AstNode *node = malloc(sizeof(AstNode));
  if (node == NULL) {
    return NULL;
  }
  switch (value->type) {
  case TOKEN_INT_LIT:
    node->kind = AST_INT_LIT;
    node->data.int_value = value->attr.int_val;
    break;
  case TOKEN_FLOAT_LIT:
    node->kind = AST_FLOAT_LIT;
    node->data.float_value;
    break;
  case TOKEN_STRING_LIT:
    node->kind = AST_STRING_LIT;
    node->data.int_value = value->attr.str_val;
    break;
  case TOKEN_IDENTIFIER:
    node->kind = AST_IDENTIFIER;
    node->data.int_value = value->attr.str_val;
    break;
  default:
    free(node);
    return NULL;
  }
  return node;
}