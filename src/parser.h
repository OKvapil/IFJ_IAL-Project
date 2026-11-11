/**
 * @file parser.h
 * @brief Syntax analysis of IFJ26 source code
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

/**
 * @brief Entry point of the syntax analyzer. Reads the first token,
 *        parses the entire IFJ26 program, and returns the root AST node.
 *
 * @return Pointer to the root AST_BLOCK node of the program.
 */
AstNode *parser_parse(void);

#endif // PARSER_H
