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
  AST_VAL,
  AST_VAR,
  AST_BOOL_LIT,
  AST_FOR,
  AST_UNIT_LIT
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
    bool bool_value;

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
      struct AstNode **params;
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
      struct AstNode *expression;
    } return_state;

    struct {
      char *var_name;
      struct AstNode *from;
      struct AstNode *to;
      struct AstNode *step;
      bool until;
      struct AstNode **guards;
      size_t guard_count;
      struct AstNode *body;
    } for_state;
  } data;
} AstNode;

/**
 * @brief Creates a new AST node for an if-then-else statement.
 *
 * @param cond Condition expression node.
 * @param then_block Block of statements executed when condition is true.
 * @param else_block Block of statements executed when condition is false.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_if(AstNode *cond, AstNode *then_block, AstNode *else_block,
                size_t line);

/**
 * @brief Creates a new AST node for a while-do loop.
 *
 * @param cond Loop condition expression node.
 * @param block Block of statements representing the loop body.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_while(AstNode *cond, AstNode *block, size_t line);

/**
 * @brief Creates a new AST node for a variable assignment statement.
 *
 * @param name Name of the target variable (takes ownership of the string).
 * @param value Assigned expression node.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_expr_assign(char *name, AstNode *value, size_t line);

/**
 * @brief Creates a new AST node for a binary operation.
 *
 * @param op Operator token type.
 * @param left Left operand expression node.
 * @param right Right operand expression node.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_binary(TokenType op, AstNode *left, AstNode *right, size_t line);

/**
 * @brief Creates a new AST node for a unary operation.
 *
 * @param op Unary operator token type.
 * @param operand Operand expression node.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_unary(TokenType op, AstNode *operand, size_t line);

/**
 * @brief Creates a leaf AST node (literal or identifier) from a scanner token.
 *
 * @param value Pointer to the source token (transfers ownership of str_val if
 * present).
 * @return Pointer to the newly allocated AstNode, or NULL on unsupported token
 * type.
 */
AstNode *ast_from_token(Token *value);

/**
 * @brief Creates an empty AST block node for holding a sequence of statements.
 *
 * @return Pointer to the newly allocated block AstNode.
 */
AstNode *ast_block(void);

/**
 * @brief Appends a statement node to an existing AST block node.
 *
 * @param block Target block node (must have kind AST_BLOCK).
 * @param statement Statement node to append.
 * @return Pointer to the updated block node, or NULL if arguments are NULL.
 */
AstNode *ast_block_add_stmt(AstNode *block, AstNode *statement);

/**
 * @brief Creates a new AST node for a variable or constant declaration (val /
 * var).
 *
 * @param kind Declaration kind (AST_VAL or AST_VAR).
 * @param name Identifier name (takes ownership of the string).
 * @param type Annotated data type (or TYPE_UNKNOWN if omitted).
 * @param init_expr Initializer expression node.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_var_decl(AstKind kind, char *name, DataType type,
                      AstNode *init_expr, size_t line);

/**
 * @brief Creates a new AST node for a function definition.
 *
 * @param name Function identifier name (takes ownership of the string).
 * @param return_type Declared return data type.
 * @param params Array of parameter AST nodes.
 * @param param_count Number of parameters.
 * @param block Function body (block or single expression node).
 * @param is_main True if the function is annotated with @main.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_func_def(char *name, DataType return_type, AstNode **params,
                      size_t param_count, AstNode *block, bool is_main,
                      size_t line);
/**
 * @brief Creates a new AST node for a range-based for loop (CONTROL extension).
 *
 * @param line Source line number.
 * @param name Loop control variable name (takes ownership of the string).
 * @param from Start expression of the range.
 * @param to End expression of the range.
 * @param step Step expression after "by" (or NULL if omitted).
 * @param until True for "until" (exclusive upper bound), false for "to"
 * (inclusive).
 * @param guards Array of guard condition expressions ("if ...").
 * @param guard_count Number of guard conditions.
 * @param body Loop body (block or single statement/expression).
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_for(size_t line, char *name, AstNode *from, AstNode *to,
                 AstNode *step, bool until, AstNode **guards,
                 size_t guard_count, AstNode *body);


/**
 * @brief Creates a new node that has result_type UNIT insted of UNKNOWN
 *
 * @param line line number
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_unit(size_t line);

/**
 * @brief Creates a new AST node for a function call.
 *
 * @param name Called function identifier name (takes ownership of the string).
 * @param args Array of argument expression nodes.
 * @param arg_count Number of arguments.
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_func_call(char *name, AstNode **args, size_t arg_count,
                       size_t line);

/**
 * @brief Recursively frees an AST node and all of its children and owned data.
 *
 * @param node Root of the AST subtree to free (may be NULL).
 * @return Always returns NULL.
 */
AstNode *ast_free(AstNode *node);

/**
 * @brief Creates a new AST node for a return statement.
 *
 * @param expr Returned expression node (or NULL for empty return).
 * @param line Source line number.
 * @return Pointer to the newly allocated AstNode.
 */
AstNode *ast_return(AstNode *expr, size_t line);
#endif
