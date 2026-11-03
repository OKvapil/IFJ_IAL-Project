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
#include "types.h"

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

typedef struct AstNode {
  AstKind kind;
  size_t line;
  DataType result_type;
  int var_id;

  union {
    int64_t int_value;
    double float_value;
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

AstNode *ast_new_if(AstNode *cond, AstNode *then_block, AstNode *else_block,
                    size_t line);
AstNode *ast_new_while(AstNode *cond, AstNode *block, size_t line);
AstNode *ast_new_expr_assign(char *name, AstNode *value, size_t line);
AstNode *ast_new_binary(TokenType op, AstNode *left, AstNode *right,
                        size_t line);
AstNode *ast_new_unary(TokenType op, AstNode *operand, size_t line);
AstNode *ast_new_from_token(Token *value);

#endif