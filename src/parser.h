/**
 * @file parser.h
 * @brief IFJcode26 header file for parser function
 *        that checks token  (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */

#ifndef PARSER_H
#define PARSER_H
#include "token.h"

void check_token(TokenType expected_token);

#endif // PARSER_H