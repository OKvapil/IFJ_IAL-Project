/**
 * @file ast.h
 * @brief Implementation of abstract syntax tree header
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */
#ifndef AST_H
#define AST_H

#include "token.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
  AST_INT_LIT,
  AST_DOUBLE_LIT,
  AST_STRING_LIT,
  AST_IDENTIFIER,
  AST_UNARY,
  AST_BINARY,
  AST_FUNC_DEF,
  AST_IF,
  AST_WHILE,
  AST_BLOCK,
  AST_ASSIGN,
  AST_THEN,
  AST_VAL,
  AST_VAR
} AstKind;

typedef struct {
  AstKind kind;
  size_t line;
  int result_type;

  union {
    int64_t int_value;
    double double_value;
    char *text;

    struct {
      TokenType op;
      struct AstNode *operand;
    } unary;
    struct {
      TokenType op;
      struct AstNode *left;
      struct AstNode *right;
    } binary;
    struct {
      struct AstNode *cond;
      struct AstNode *then_block;
      struct AstNode *else_block;
    } if_state;
    struct {
      struct AstNode *cond;
      struct AstNode *block;
    } while_state;
    struct {
      char *name;
      struct AstNode *value;
    } expr_assign;
  } data;
} AstNode;

#endif