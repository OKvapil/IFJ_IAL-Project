#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>
#include <stddef.h>

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

    /* --- Identifiers and Literals --- */
    TOKEN_IDENTIFIER,       // Variable or function name
    TOKEN_INT_LIT,          // Integer literal
    TOKEN_FLOAT_LIT,        // Double literal
    TOKEN_STRING_LIT,       // String literal 
    TOKEN_UNIT_LIT,         // Unit literal ()

    /* --- Annotations --- */
    TOKEN_ANNOT_MAIN,       // '@main' (entry point function marker)
    
    /* --- Assignment and Arithmetic Operators --- */
    TOKEN_ASSIGN,           // '=' (assignment)
    TOKEN_PLUS,             // '+' (addition or string concatenation)
    TOKEN_MINUS,            // '-' (subtraction or unary minus)
    TOKEN_MUL,              // '*' (multiplication)
    TOKEN_DIV,              // '/' (division)

    /* --- Relational / Comparison Operators --- */
    TOKEN_EQ,               // '==' (equality comparison)
    TOKEN_NEQ,              // '!=' (inequality comparison)
    TOKEN_LT,               // '<'  (less than)
    TOKEN_LTE,              // '<=' (less than or equal)
    TOKEN_GT,               // '>'  (greater than)
    TOKEN_GTE,              // '>=' (greater than or equal)

    /* --- Delimiters and Punctuation --- */
    TOKEN_LPAREN,           // '(' (opening parenthesis)
    TOKEN_RPAREN,           // ')' (closing parenthesis)
    TOKEN_COLON,            // ':' (type annotation separator)
    TOKEN_COMMA,            // ',' (parameter or argument separator)
    TOKEN_SEMICOLON,        // ';' (statement separator on the same line)
    TOKEN_DOT,              // '.' (used in import prologue 'import ifj26.*')
    
} TokenType;