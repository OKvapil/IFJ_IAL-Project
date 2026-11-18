/**
 * @file semantic.h
 * @brief Semantic analysis header file.
 * 
 * IFJ/IAL project - IFJ26 compiler
 * 
 * @author Jonáš Ľalík (xlalikj00)
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "errors.h"

/**
 * @brief Performs the semantic analysis of a whole program.
 * 
 * Creates the symbol table and frees it before returning.
 * Stops at the first error found.
 * 
 * @param root The root node of the AST.
 * @return ERR_OK if okay, corresponding error code if an error occurs.
 */
ErrorCode sem_analyze(AstNode *root);

#endif //SEMANTIC_H