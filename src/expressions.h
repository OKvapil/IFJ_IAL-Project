/**
 * @file expressions.h
 * @brief IFJ26 parser_expression header file
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ast.h"
#include "token.h"

/**
 * @brief Parses an expression using bottom-up operator precedence analysis
 *        and builds its corresponding AST subtree.
 *
 * @param token Pointer to the current lookahead token (holds the first token of
 * the expression on entry, and the first token following the expression on
 * return).
 * @return Pointer to the root AstNode of the parsed expression (exits with
 * error code ERR_LEXICAL on lexical error, ERR_SYNTAX on syntax error, or
 * ERR_INTERNAL on internal allocation error).
 */
AstNode *parse_expression(Token *token);
/**
 * @brief Continues parsing an expression whose first operand is already built.
 *
 * @param token Pointer to the current lookahead token (holds the first token
 * after @p first_operand on entry, and the first token following the expression
 * on return).
 * @param first_operand Already built AST node of the first operand (takes
 * ownership).
 * @return Pointer to the root AstNode of the parsed expression (exits with
 * error code ERR_LEXICAL on lexical error, ERR_SYNTAX on syntax error, or
 * ERR_INTERNAL on internal allocation error).
 */
AstNode *parse_expression_from(Token *token, AstNode *first_operand);

#endif // EXPRESSION_H