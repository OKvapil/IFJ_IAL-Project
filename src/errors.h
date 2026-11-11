/**
 * @file errors.h
 * @brief Error codes data type enum.
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Jonáš Ľalík (xlalikj00)
 */

#ifndef ERRORS_H
#define ERRORS_H

/**
 * @brief Compiler error codes.
 */
typedef enum {
  ERR_OK = 0,      ///< No error.
  ERR_LEXICAL = 1, ///< Lexical analysis error.
  ERR_SYNTAX = 2,  ///< Syntax analysis error.
  ERR_UNDEFINED =
      3, ///< Semantic error - use of an undefined function, variable or type;
         ///< missing function with the \@main annotation.
  ERR_REDEFINITION = 4, ///< Semantic error - disallowed redefinition of a
                        ///< function, variable or type.
  ERR_CALL_ARGS =
      5, ///< Semantic error - wrong number or type of arguments in a function
         ///< call; disallowed parameters of \@main function.
  ERR_TYPE = 6, ///< Semantic error - type incompatibility or type inference
                ///< failure in an expression, assignment, initialization,
                ///< return value or control construct.
  ERR_IMMUTABLE = 7, ///< Semantic error - writing to an unmodifiable variable
                     ///< or a function parameter.
  ERR_OTHER_SEMANTIC =
      10, ///< Other semantic errors (for example a disallowed return).
  ERR_INTERNAL =
      99 ///< Internal compiler error (for example an allocation failure).
} ErrorCode;

#endif // ERRORS_H