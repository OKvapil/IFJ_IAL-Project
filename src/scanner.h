/**
 * @file scanner.h
 * @brief Lexical analyzer interface for the IFJ26 compiler
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Filip Haza (xhazafi00)
 */

#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <stdio.h>

/**
 * @brief Sets the input source stream for lexical analysis (stdin)
 */
void scanner_init(FILE *source);

/**
 * @brief Reads the next token from input
 *
 * @param token pointer to the Token structure
 * @return int 0 on succes, 1 on lexical error
 */
int scanner_get_next_token(Token *token);

/**
 * @brief Deallocates internal scanner resources (e.g. indentation stack).
 */
void scanner_cleanup(void);

#endif // SCANNER_H