#ifndef SCANNER_H
#define SCANNER_H

#include<stdio.h>
#include "token.h"

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