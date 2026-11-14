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
 * @brief Generates code that leaves the value of an expression on the stack.
 *
 * Handles literals (int@, float@, string@, bool@), variables
 * (LF@<name>$<var_id>), unary and binary operations, function calls and if
 * expressions.
 *
 * @param node Expression node.
 *
 * @return true on success, false on an invalid or unsupported node
 *         (compiler ends with 99).
 */
bool gen_expr(const AstNode *node);

/**
 * @brief Generates code for one statement; it leaves nothing on the stack.
 *
 * Handles assignment, val/var declaration, while, for, if, block and
 * return. An expression or call used as a statement is evaluated and its
 * value is dropped into GF@$trash.
 *
 * @param node Statement node.
 *
 * @return true on success, false on an invalid or unsupported node
 *         (compiler ends with 99).
 */
bool gen_statement(const AstNode *node);

/**
 * @brief Generates the start of the program.
 *
 * Prints the header, defines the global helper variables, calls the main
 * function and ends the program with EXIT.
 *
 * @param main_name Name of the function marked with @main.
 *
 * @return true on success, false if the name is NULL or too long.
 */
bool gen_program_start(const char *main_name);

/**
 * @brief Generates the end of the program.
 *
 * Prints the definitions of the built-in functions implemented as IFJcode26
 * subroutines (subStr, ord).
 */
void gen_program_end(void);

/**
 * @brief Generates the whole IFJcode26 program from the AST root.
 *
 * Finds the function marked with @main, generates the program start
 * (header, global helper variables, call of main, exit), then all function
 * definitions and finally the definitions of the built-in functions
 * implemented in IFJcode26 (subStr, ord).
 *
 * @param root Root of the AST: an AST_BLOCK containing only AST_FUNC_DEF nodes,
 * as returned by the parser.
 * @return true on success; false if the tree is invalid (root is not a block,
 * contains something other than a function definition, has no main function) or
 * generation of any function failed.
 */
bool gen_program(const AstNode *root);
#endif
