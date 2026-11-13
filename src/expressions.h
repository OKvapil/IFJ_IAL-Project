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

#endif // EXPRESSION_H