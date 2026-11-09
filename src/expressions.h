/**
 * @file parser.c
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

AstNode *parse_expression(Token *token);

#endif // EXPRESSION_H