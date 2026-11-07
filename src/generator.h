/**
 * @file generator.h
 * @brief Generates IFJcode26 from the abstract syntax tree (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#ifndef IFJ_IAL_PROJECT_GENERATOR_H
#define IFJ_IAL_PROJECT_GENERATOR_H

#include "ast.h"
#include <stdbool.h>

/**
 * @brief Generates code that leaves the value of an expression on the stack
 *
 * Literals are pushed as constants (int@, float@, string@), variables as
 * LF@<text>$<var_id>
 *
 * @param node Expression node
 *
 * @return true on success, false on allocation error (compiler ends with 99)
 */
bool gen_expr(const AstNode *node);

/**
 * @brief Generates code for one statement
 *
 * @param node Statement node
 *
 * @return true on success, false on allocation error (compiler ends with 99)
 */
bool gen_statement(const AstNode *node);

/**
 * @brief Generates start of the program
 *
 * Prints the header, defines global variables, calls the main function and ends
 * the program
 *
 * @param main_name Name of the function
 *
 * @return false if the name is too long or NULL
 */
bool gen_program_start(const char *main_name);

/**
 * @brief Generates end of the program
 *
 * Prints the definitions of the build-in functions implemented as IFJcode26
 */
void gen_program_end(void);
#endif
