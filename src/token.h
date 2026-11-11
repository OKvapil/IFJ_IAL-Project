/**
 * @file token.h
 * @brief Token definitions and data structures for the IFJ26 compiler
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Filip Haza (xhazafi00)
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
  /* --- Control and Layout Tokens --- */
  TOKEN_EOF,
  TOKEN_EOL,
  TOKEN_INDENT,
  TOKEN_DEDENT,

  /* --- Keywords --- */
  TOKEN_KW_DEF,
  TOKEN_KW_DO,
  TOKEN_KW_DOUBLE,
  TOKEN_KW_ELSE,
  TOKEN_KW_IF,
  TOKEN_KW_IMPORT,
  TOKEN_KW_INT,
  TOKEN_KW_RETURN,
  TOKEN_KW_STRING,
  TOKEN_KW_THEN,
  TOKEN_KW_UNIT,
  TOKEN_KW_VAL,
  TOKEN_KW_VAR,
  TOKEN_KW_WHILE,
  // BOOLTHEN EXTENSION
  TOKEN_KW_TRUE,
  TOKEN_KW_FALSE,
  TOKEN_KW_BOOLEAN,
  // CONTROL EXTENSION
  TOKEN_KW_TO,
  TOKEN_KW_UNTIL,
  TOKEN_KW_BY,
  TOKEN_KW_FOR,

  /* --- Control extension --- */
  TOKEN_ARROW, // '<-'

  /* --- Identifiers and Literals --- */
  TOKEN_IDENTIFIER, // Variable or function name
  TOKEN_INT_LIT,    // Integer literal
  TOKEN_DOUBLE_LIT, // Double literal
  TOKEN_STRING_LIT, // String literal

  /* --- Annotations --- */
  TOKEN_ANNOT_MAIN, // '@main' (entry point function marker)

  /* --- Assignment and Arithmetic Operators --- */
  TOKEN_ASSIGN, // '=' (assignment)
  TOKEN_PLUS,   // '+' (addition or string concatenation)
  TOKEN_MINUS,  // '-' (subtraction or unary minus)
  TOKEN_MUL,    // '*' (multiplication)
  TOKEN_DIV,    // '/' (division)

  /* --- Relational / Comparison Operators --- */
  TOKEN_EQ,  // '==' (equality comparison)
  TOKEN_NEQ, // '!=' (inequality comparison)
  TOKEN_LT,  // '<'  (less than)
  TOKEN_LTE, // '<=' (less than or equal)
  TOKEN_GT,  // '>'  (greater than)
  TOKEN_GTE, // '>=' (greater than or equal)

  /* --- Logical operators, boolthen extension --- */
  TOKEN_NOT, // '!'
  TOKEN_AND, // '&&'
  TOKEN_OR,  //'||'

  /* --- Delimiters and Punctuation --- */
  TOKEN_LPAREN,    // '(' (opening parenthesis)
  TOKEN_RPAREN,    // ')' (closing parenthesis)
  TOKEN_COLON,     // ':' (type annotation separator)
  TOKEN_COMMA,     // ',' (parameter or argument separator)
  TOKEN_SEMICOLON, // ';' (statement separator on the same line)
  TOKEN_DOT,       // '.' (used in import prologue 'import ifj26.*')

} TokenType;

/**
 * @brief Semantic value held by a token depending on its type.
 */
typedef union {
  int64_t int_val;   // For TOKEN_INT_LIT (64-bit signed integer)
  double double_val; // For TOKEN_DOUBLE_LIT (64-bit IEEE 754 floating-point)
  char *str_val; // For TOKEN_IDENTIFIER and TOKEN_STRING_LIT (heap-allocated
                 // string)
} TokenAttribute;

/**
 * @brief Complete token structure passed from scanner to parser.
 */
typedef struct {
  TokenType type;      // Token category from the enum above
  TokenAttribute attr; // Semantic value (if required by the token type)
  size_t line;         // Source line number for syntax/semantic error reporting
} Token;

/**
 * @brief Deallocates dynamically allocated memory inside token attribute (e.g.
 * str_val).
 *
 * @param tok Pointer to the token to clean up.
 */
void token_clear(Token *tok);

#endif // TOKEN_H