/**
 * @file expressions.h
 * @brief IFJcode26 parser_expression header file (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "token.h"
#include "ast.h"

/**
 * @brief Parses an expression using bottom-up operator precedence analysis
 *        and builds its corresponding AST subtree.
 *
 * @param token Pointer to the current lookahead token (holds the first token of the
 *              expression on entry, and the first token following the expression on return).
 * @return Pointer to the root AstNode of the parsed expression (exits with error code
 *         1 on lexical error, 2 on syntax error, or 99 on internal allocation error).
 */
AstNode *parse_expression(Token *token);

#endif // EXPRESSION_H