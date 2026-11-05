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


#endif
