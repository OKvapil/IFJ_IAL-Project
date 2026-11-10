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
#include "types.h"
#include <stdbool.h>
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
  AST_FUNC_CALL,
  AST_RETURN,
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

    struct {
      char *name;
      struct AstNode **args;
      size_t arg_count;
    } func_call;

    struct {
      struct AstNode **statements;
      size_t count;
      size_t capacity;
    } block;

    struct {
      char *name;
      struct AstNode *params;
      size_t param_count;
      DataType return_type;
      struct AstNode *block;
      bool is_main;
    } func_def;

    struct {
      DataType var_type;
      char *name;
      struct AstNode *init_expr;
    } var_dec;

    struct {
      DataType var_type;
      char *name;
    } params;

    struct {
      struct AstNode *expression;
    } return_state;
  } data;
} AstNode;

AstNode *ast_if(AstNode *cond, AstNode *then_block, AstNode *else_block,
                size_t line);
AstNode *ast_while(AstNode *cond, AstNode *block, size_t line);
AstNode *ast_expr_assign(char *name, AstNode *value, size_t line);
AstNode *ast_binary(TokenType op, AstNode *left, AstNode *right, size_t line);
AstNode *ast_unary(TokenType op, AstNode *operand, size_t line);
AstNode *ast_from_token(Token *value);
AstNode *ast_block(void);
AstNode *ast_block_add_stmt(AstNode *block, AstNode *statement);
AstNode *ast_var_decl(AstKind kind, char *name, DataType type,
                      AstNode *init_expr, size_t line);

AstNode *ast_func_def(char *name, DataType return_type, AstNode *params,
                      size_t param_count, AstNode *block, bool is_main,
                      size_t line);
AstNode *ast_func_call(char *name, AstNode **args, size_t arg_count,
                       size_t line);
AstNode *ast_free(AstNode *node);
AstNode *ast_return(AstNode *expr, size_t line);

#endif
