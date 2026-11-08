/**
 * @file scanner.c
 * @brief Lexical analyzer implementation for the IFJ26 compiler
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Filip Haza (xhazafi00)
 */

#include "scanner.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR_INTERNAL 99
#define ERR_LEXICAL 1
#define BUFFER_INIT_CAPACITY 32

static FILE *source_file = NULL;
static size_t current_line = 1;
static int bracket_depth = 0;
static bool at_line_start = true;
static size_t space_count = 0;
static bool has_tab = false;
static int comment_depth = 0;

/* ============================================================================
 * Authorship Note:
 * The following data structure `scanner_buffer_t` and its static utility
 * functions were designed with the assistance of AI (Google Gemini 3.1 Pro)
 * based on the following prompt:
 *
 * "V jazyce C navhrni strukturu pamětově bezpečného dynamického bufferu
 * určeného pro lexikální analyzátor překladače. Struktura bude uchovávat
 * ukazatel na data, aktuální délku a kapacitu. Doplň také pomocné funkce
 * pro inicializaci, postupné přidávání znaků, resetování délky pro nový token.
 * případné selhání malloc/realloc musí být ohlášeno s návratovým kodem 99.
 * Předem si projdi zadání projektu a strukturu jazyka ifj26 zkontoluj zda vše
 * odpovídá zadání a struktuře jazyka ifj26. Pouzivej anglické kometnáře,
 * snake_case.
 *
 * Pokud ti neco není jasné doptávej se."
 * ============================================================================
 */

// ---------- Dynamic buffer
typedef struct {
  char *data;
  size_t length;
  size_t capacity;
} scanner_buffer_t;

static scanner_buffer_t s_buf;

static void scanner_buffer_init(scanner_buffer_t *buf) {
  buf->capacity = BUFFER_INIT_CAPACITY;
  buf->length = 0;
  buf->data = (char *)malloc(buf->capacity);
  if (!buf->data) {
    fprintf(stderr,
            "Internal error: Memory allocation failed for scanner buffer.\n");
    exit(ERR_INTERNAL);
  }
  buf->data[0] = '\0';
}

static void scanner_buffer_append(scanner_buffer_t *buf, char ch) {
  if (buf->length + 1 >= buf->capacity) {
    size_t new_capacity = buf->capacity * 2;
    char *new_data = (char *)realloc(buf->data, new_capacity);
    if (!new_data) {
      free(buf->data);
      fprintf(
          stderr,
          "Internal error: Memory reallocation failed for scanner buffer.\n");
      exit(ERR_INTERNAL);
    }
    buf->data = new_data;
    buf->capacity = new_capacity;
  }

  buf->data[buf->length++] = ch;
  buf->data[buf->length] = '\0';
}

static void scanner_buffer_clear(scanner_buffer_t *buf) {
  buf->length = 0;
  if (buf->data) {
    buf->data[0] = '\0';
  }
}

static void scanner_buffer_free(scanner_buffer_t *buf) {
  if (buf->data) {
    free(buf->data);
    buf->data = NULL;
  }
  buf->length = 0;
  buf->capacity = 0;
}

// ---------- keyword struct
typedef struct {
  const char *keyword;
  TokenType type;
} keyword_entry_t;

static const keyword_entry_t KEYWORDS[] = {
    {"def", TOKEN_KW_DEF},       {"do", TOKEN_KW_DO},
    {"Double", TOKEN_KW_DOUBLE}, {"else", TOKEN_KW_ELSE},
    {"if", TOKEN_KW_IF},         {"import", TOKEN_KW_IMPORT},
    {"Int", TOKEN_KW_INT},       {"return", TOKEN_KW_RETURN},
    {"String", TOKEN_KW_STRING}, {"then", TOKEN_KW_THEN},
    {"Unit", TOKEN_KW_UNIT},     {"val", TOKEN_KW_VAL},
    {"var", TOKEN_KW_VAR},       {"while", TOKEN_KW_WHILE},
};

// ---------- DFA states
typedef enum scanner_state_e {
  STATE_START, // Initial state

  /* --- Keywords and Identifiers --- */
  STATE_KW_IDENTIFIER, // Final: keyword or id
  STATE_UNDERSCORE,    // Wait: alnum or '_' after '_'

  /* --- Numbers --- */
  STATE_INT,               // Final: integer
  STATE_INT_UNDERSCORE,    // Wait: digit after '_'
  STATE_EXPONENT,          // Wait: 'e' or 'E'
  STATE_DOUBLE,            // Final: float sequence
  STATE_DOUBLE_UNDERSCORE, // Wait: digit after '_' in double
  STATE_NUM_DOT,           // Wait: '.' in number
  STATE_D_SUFIX,           // Final: double suffix
  STATE_SIGN,              // Wait: '+' or '-' in exp
  STATE_EXPONENT_FINAL,    // Final: digits after exp
  STATE_EXP_UNDERSCORE,    // Wait: digit after '_' in exp
  STATE_ZERO,              // Final: standalone '0'
  STATE_DOT_CHECK,         // Wait: check if dot is number

  /* --- Strings --- */
  STATE_QUOTE_1,       // Wait: 1st '"'
  STATE_STRING,        // In string literal
  STATE_STRING_ESCAPE, // Wait: '\' escape
  STATE_QUOTE_2,       // Wait/Final: 2nd '"'
  STATE_MSTR,          // In multi-line (""")
  STATE_MSTR_END_1,    // Wait: 1st '"' of """
  STATE_MSTR_END_2,    // Wait: 2nd '"' of """
  STATE_MSTR_END_3,    // Final: closed """

  /* --- Operators and Delimiters --- */
  STATE_ASSIGN,       // Decision for '=' or '=='
  STATE_LESS_THAN,    // Decision for '<' or '<='
  STATE_GREATER_THAN, // Decision for '>' or '>='

  STATE_PLUS,     // Final: '+'
  STATE_MINUS,    // Final: '-'
  STATE_MULTIPLY, // Final: '*'

  STATE_NOT, // Decision for '!' / '!='

  /* --- Division and Comments --- */
  STATE_SLASH,           // Wait: '/'
  STATE_COM_LINE,        // In line comment
  STATE_COM_BLOCK,       // In block comment
  STATE_COM_BLOCK_SLASH, // Wait: '*' for nested block comment
  STATE_COM_BLOCK_STAR,  // Wait: '/' in block comment

  /* --- Punctuation and Brackets --- */
  STATE_SEMICOLON, // Final: ';'
  STATE_COLON,     // Final: ':'
  STATE_COMMA,     // Final: ','
  STATE_LPAR,      // Final: '('
  STATE_RPAR,      // Final: ')'

  /* --- Annotation main --- */
  STATE_ANNOT,
  STATE_ANNOT_M,
  STATE_ANNOT_MA,
  STATE_ANNOT_MAI,

  /* --- End of File/Line --- */
  STATE_EOL, // Final: End of line
  STATE_EOF, // Final: End of File

} scanner_state_t;

void scanner_init(FILE *source) {
  if (source != NULL) {
    source_file = source;
  } else {
    source_file = stdin;
  }
  current_line = 1;
  bracket_depth = 0;
  at_line_start = true;
  space_count = 0;
  has_tab = false;
  scanner_buffer_init(&s_buf);
}

void scanner_cleanup(void) {
  scanner_buffer_free(&s_buf);
  source_file = NULL;
  space_count = 0;
  has_tab = false;
}

int scanner_get_next_token(Token *token) {

  // check if token is passed right from parser
  if (token == NULL) {
    fprintf(stderr, "Internal error: Null pointer passed as token.\n");
    return (ERR_INTERNAL);
  }

  // cleanup before next token
  scanner_buffer_clear(&s_buf);

  scanner_state_t state = STATE_START;
  int c;

  while (true) {
    c = fgetc(source_file);

    switch (state) {

    /* --- PUNCTUATION AND BRACKETS --- */
    case STATE_SEMICOLON:
      ungetc(c, source_file);
      token->type = TOKEN_SEMICOLON;
      token->line = current_line;
      return 0;

    case STATE_COLON:
      ungetc(c, source_file);
      token->type = TOKEN_COLON;
      token->line = current_line;
      return 0;

    case STATE_COMMA:
      ungetc(c, source_file);
      token->type = TOKEN_COMMA;
      token->line = current_line;
      return 0;

    case STATE_LPAR:
      ungetc(c, source_file);
      token->type = TOKEN_LPAREN;
      token->line = current_line;
      return 0;

    case STATE_RPAR:
      ungetc(c, source_file);
      token->type = TOKEN_RPAREN;
      token->line = current_line;
      return 0;

    /* --- END OF LINE / FILE --- */
    case STATE_EOL:
      ungetc(c, source_file);
      token->type = TOKEN_EOL;
      token->line = current_line - 1;
      return 0;

    case STATE_EOF:
      token->type = TOKEN_EOF;
      token->line = current_line;
      return 0;

    /* --- ARITHMETIC OPERATORS --- */
    case STATE_PLUS:
      ungetc(c, source_file);
      token->type = TOKEN_PLUS;
      token->line = current_line;
      return 0;

    case STATE_MINUS:
      ungetc(c, source_file);
      token->type = TOKEN_MINUS;
      token->line = current_line;
      return 0;

    case STATE_MULTIPLY:
      ungetc(c, source_file);
      token->type = TOKEN_MUL;
      token->line = current_line;
      return 0;

    /* --- RELATIONAL AND ASSIGNMENT OPERATORS --- */
    case STATE_GREATER_THAN:
      if (c == '=') {
        token->type = TOKEN_GTE;
        token->line = current_line;
        return 0;
      } else {
        ungetc(c, source_file);
        token->type = TOKEN_GT;
        token->line = current_line;
        return 0;
      }

    case STATE_LESS_THAN:
      if (c == '=') {
        token->type = TOKEN_LTE;
        token->line = current_line;
        return 0;
      } else {
        ungetc(c, source_file);
        token->type = TOKEN_LT;
        token->line = current_line;
        return 0;
      }

    case STATE_ASSIGN:
      if (c == '=') {
        token->type = TOKEN_EQ;
        token->line = current_line;
        return 0;
      } else {
        ungetc(c, source_file);
        token->type = TOKEN_ASSIGN;
        token->line = current_line;
        return 0;
      }

    case STATE_NOT:
      if (c == '=') {
        token->type = TOKEN_NEQ;
        token->line = current_line;
        return 0;
      } else {
        return ERR_LEXICAL;
      }

    /* --- ANNOTATION MAIN --- */
    case STATE_ANNOT:
      if (c == 'm')
        state = STATE_ANNOT_M;
      else
        return ERR_LEXICAL;
      break;

    case STATE_ANNOT_M:
      if (c == 'a')
        state = STATE_ANNOT_MA;
      else
        return ERR_LEXICAL;
      break;

    case STATE_ANNOT_MA:
      if (c == 'i')
        state = STATE_ANNOT_MAI;
      else
        return ERR_LEXICAL;
      break;

    case STATE_ANNOT_MAI:
      if (c == 'n') {
        token->type = TOKEN_ANNOT_MAIN;
        token->line = current_line;
        return 0;
      } else {
        return ERR_LEXICAL;
      }

    /* --- DIVISION AND COMMENTS --- */
    case STATE_SLASH:
      if (c == '/') {
        state = STATE_COM_LINE;
        break;
      } else if (c == '*') {
        comment_depth++;
        state = STATE_COM_BLOCK;
        break;
      } else {
        at_line_start = false;
        ungetc(c, source_file);
        token->type = TOKEN_DIV;
        token->line = current_line;
        return 0;
      }

    case STATE_COM_LINE:
      if (c == EOF) {
        state = STATE_EOF;
        break;
      } else if (c == '\n') {
        current_line++;

        // empty line
        if (at_line_start == true) {
          state = STATE_START;
          space_count = 0;
          has_tab = false;
          break;

          // non empty line
        } else {
          at_line_start = true;
          space_count = 0;
          has_tab = false;
          if (bracket_depth <= 0) {
            state = STATE_EOL;
            break;

            // inside brackets, LF is ignored
          } else {
            state = STATE_START;
            break;
          }
        }
        // comment body
      } else {
        break;
      }

    case STATE_COM_BLOCK:
      if (c == EOF) {
        return ERR_LEXICAL; // Unclosed block comment
      } else if (c == '*') {
        state = STATE_COM_BLOCK_STAR; // Potential comment close
        break;
      } else if (c == '/') {
        state = STATE_COM_BLOCK_SLASH; // Potential nested comment start
        break;
      } else if (c == '\n') {
        current_line++;
        break;
      }
      break;

    case STATE_COM_BLOCK_SLASH:
      if (c == '*') {
        comment_depth++; // Nested comment detected (/*)
        state = STATE_COM_BLOCK;
        break;
      } else if (c == '/') {
        state = STATE_COM_BLOCK_SLASH; // Multiple slashes (e.g. ///)
        break;
      } else if (c == '\n') {
        current_line++;
        state = STATE_COM_BLOCK;
        break;
      } else if (c == EOF) {
        return ERR_LEXICAL;
      } else {
        state = STATE_COM_BLOCK;
        break;
      }

    case STATE_COM_BLOCK_STAR:
      if (c == '/') {
        comment_depth--; // Closed current comment level (*/)
        if (comment_depth == 0) {
          state = STATE_START; // All nested levels closed
          break;
        } else {
          state = STATE_COM_BLOCK;
          break;
        }
      } else if (c == '*') {
        break; // Multiple stars (e.g. /***)
      } else if (c == EOF) {
        return ERR_LEXICAL;
      } else if (c == '\n') {
        current_line++;
        state = STATE_COM_BLOCK;
        break;
      } else {
        state = STATE_COM_BLOCK;
        break;
      }

    /* --- IDENTIFIERS AND KEYWORDS --- */
    case STATE_KW_IDENTIFIER:
      if (isalnum(c) || c == '_') {
        scanner_buffer_append(&s_buf, c);
        break;
      } else {
        ungetc(c, source_file);
        token->type = TOKEN_IDENTIFIER;
        token->line = current_line;
        for (size_t i = 0; i < sizeof(KEYWORDS) / sizeof(KEYWORDS[0]); i++) {
          if (strcmp(KEYWORDS[i].keyword, s_buf.data) == 0) {
            token->type = KEYWORDS[i].type;
            return 0;
          }
        }
        token->attr.str_val = strdup(s_buf.data);
        return 0;
      }

    case STATE_UNDERSCORE:
      if (isalnum(c) || c == '_') {
        scanner_buffer_append(&s_buf, c);
        state = STATE_KW_IDENTIFIER;
        break;
      } else {
        return ERR_LEXICAL;
      }

    /* --- NUMBERS --- */
    case STATE_ZERO:
      if (c == 'e' || c == 'E') {
        // Exponent
        scanner_buffer_append(&s_buf, c);
        state = STATE_EXPONENT;
        break;
      } else if (c == '.') {
        // 0. floating point
        scanner_buffer_append(&s_buf, c);
        state = STATE_NUM_DOT;
        break;
      } else if (isdigit(c) || c == '_') {
        // ERROR: number cannot begin with 0
        return ERR_LEXICAL;
      } else {
        // Return int token
        ungetc(c, source_file);
        token->type = TOKEN_INT_LIT;
        token->line = current_line;
        token->attr.int_val = strtoll(s_buf.data, NULL, 10);
        return 0;
      }

    case STATE_INT:
      if (isdigit(c)) {
        // Digits
        scanner_buffer_append(&s_buf, c);
        break;
      } else if (c == '_') {
        // Underscore between digits
        state = STATE_INT_UNDERSCORE;
        break;
      } else if (c == 'e' || c == 'E') {
        // Exponent
        scanner_buffer_append(&s_buf, c);
        state = STATE_EXPONENT;
        break;
      } else if (c == '.') {
        // Floating point
        scanner_buffer_append(&s_buf, c);
        state = STATE_NUM_DOT;
        break;
      } else {
        // Return int token
        ungetc(c, source_file);
        token->type = TOKEN_INT_LIT;
        token->line = current_line;
        token->attr.int_val = strtoll(s_buf.data, NULL, 10);
        return 0;
      }

    case STATE_INT_UNDERSCORE:
      if (isdigit(c)) {
        scanner_buffer_append(&s_buf, c);
        state = STATE_INT;
        break;
      } else {
        return ERR_LEXICAL;
      }

    case STATE_DOT_CHECK:
      if (isdigit(c)) {
        // Double
        scanner_buffer_append(&s_buf, c);
        state = STATE_DOUBLE;
        break;
      } else {
        // Standalone dot
        ungetc(c, source_file);
        token->type = TOKEN_DOT;
        token->line = current_line;
        return 0;
      }

    case STATE_NUM_DOT:
      if (isdigit(c)) {
        // Double
        scanner_buffer_append(&s_buf, c);
        state = STATE_DOUBLE;
        break;
      } else {
        // ERROR: digit must follow dot
        return ERR_LEXICAL;
      }

    case STATE_DOUBLE:
      if (isdigit(c)) {
        // Double
        scanner_buffer_append(&s_buf, c);
        break;
      } else if (c == '_') {
        // Underscore between digits
        state = STATE_DOUBLE_UNDERSCORE;
        break;
      } else if (c == 'd' || c == 'D') {
        // D suffix
        state = STATE_D_SUFIX;
        break;
      } else if (c == 'e' || c == 'E') {
        // Exponent
        scanner_buffer_append(&s_buf, c);
        state = STATE_EXPONENT;
        break;
      } else {
        // Return double token
        ungetc(c, source_file);
        token->type = TOKEN_DOUBLE_LIT;
        token->line = current_line;
        token->attr.float_val = strtod(s_buf.data, NULL);
        return 0;
      }

    case STATE_DOUBLE_UNDERSCORE:
      if (isdigit(c)) {
        // Double
        scanner_buffer_append(&s_buf, c);
        state = STATE_DOUBLE;
        break;
      } else {
        return ERR_LEXICAL;
      }

    case STATE_EXPONENT:
      if (c == '+' || c == '-') {
        // sign after exponennt
        scanner_buffer_append(&s_buf, c);
        state = STATE_SIGN;
        break;
      } else if (isdigit(c)) {
        // number after exponent
        scanner_buffer_append(&s_buf, c);
        state = STATE_EXPONENT_FINAL;
        break;
      } else {
        // ERROR
        return ERR_LEXICAL;
      }

    case STATE_SIGN:
      // digit must be after sign
      if (isdigit(c)) {
        scanner_buffer_append(&s_buf, c);
        state = STATE_EXPONENT_FINAL;
        break;
      } else {
        return ERR_LEXICAL;
      }

    case STATE_EXPONENT_FINAL:
      if (isdigit(c)) {
        // number after exponent
        scanner_buffer_append(&s_buf, c);
        break;
      } else if (c == 'd' || c == 'D') {
        // D suffix
        state = STATE_D_SUFIX;
        break;
      } else if (c == '_') {
        // underscore in exponent
        state = STATE_EXP_UNDERSCORE;
        break;
      } else {
        ungetc(c, source_file);
        token->type = TOKEN_DOUBLE_LIT;
        token->line = current_line;
        token->attr.float_val = strtod(s_buf.data, NULL);
        return 0;
      }

    case STATE_EXP_UNDERSCORE:
      if (isdigit(c)) {
        // return to STATE_EXPONENT_FINAL
        scanner_buffer_append(&s_buf, c);
        state = STATE_EXPONENT_FINAL;
        break;
      } else {
        return ERR_LEXICAL;
      }

    case STATE_D_SUFIX:
      ungetc(c, source_file);
      token->type = TOKEN_DOUBLE_LIT;
      token->line = current_line;
      token->attr.float_val = strtod(s_buf.data, NULL);
      return 0;

    /* --- STRINGS --- */
    case STATE_QUOTE_1:
      if (c == '"') {
        // second quote
        state = STATE_QUOTE_2;
        break;
      } else if (c == '\\') {
        // escape sequence?
        state = STATE_STRING_ESCAPE;
        break;
      } else if (c == EOF || c == '\n') {
        // unterminated string literal
        return ERR_LEXICAL;
      } else {
        // line string
        scanner_buffer_append(&s_buf, c);
        state = STATE_STRING;
        break;
      }

    case STATE_STRING:
      if (c == '\\') {
        // escape sequence?
        state = STATE_STRING_ESCAPE;
        break;
      } else if (c == EOF || c == '\n') {
        // unterminated string literal
        return ERR_LEXICAL;
      } else if (c == '"') {
        token->type = TOKEN_STRING_LIT;
        token->line = current_line;
        token->attr.str_val = strdup(s_buf.data);
        return 0;
      } else {
        scanner_buffer_append(&s_buf, c);
        break;
      }

    case STATE_STRING_ESCAPE:
      switch (c) {
      case '\\':
        scanner_buffer_append(&s_buf, '\\');
        state = STATE_STRING;
        break;
      case 'n':
        scanner_buffer_append(&s_buf, '\n');
        state = STATE_STRING;
        break;
      case 'r':
        scanner_buffer_append(&s_buf, '\r');
        state = STATE_STRING;
        break;
      case 't':
        scanner_buffer_append(&s_buf, '\t');
        state = STATE_STRING;
        break;
      case '"':
        scanner_buffer_append(&s_buf, '\"');
        state = STATE_STRING;
        break;

      default:
        return ERR_LEXICAL;
      }
      break;

    case STATE_QUOTE_2:
      if (c == '"') {
        // multi line string
        state = STATE_MSTR;
        break;
      } else {
        // empty string
        ungetc(c, source_file);
        token->type = TOKEN_STRING_LIT;
        token->line = current_line;
        token->attr.str_val = strdup(s_buf.data);
        return 0;
      }

    case STATE_MSTR:
      break;

    case STATE_MSTR_END_1:
      break;

    case STATE_MSTR_END_2:
      break;

    case STATE_MSTR_END_3:
      break;

    case STATE_START:

      /* --- WHITESPACE --- */
      if (isspace(c)) {
        /* --- LF --- */
        if (c == '\n') {
          current_line++;

          /* --- EMPTY LINE --- */
          if (at_line_start) {
            space_count = 0;
            has_tab = false;
            break;
          }

          else {
            at_line_start = true;
            space_count = 0;
            has_tab = false;

            if (bracket_depth <= 0) {
              state = STATE_EOL;
              break;
            } else {
              break;
            }
          }
        }
        /* --- SPACE or TAB --- */
        if (at_line_start) {
          if (c == ' ') {
            space_count++;
            break;
          } else if (c == '\t') {
            has_tab = true;
            break;
          } else {
            break;
          }
        } else {
          break;
        }
      }

      /* --- IDENTIFIERS AND KEYWORDS --- */
      else if (isalpha(c) || c == '_') {
        at_line_start = false;
        scanner_buffer_append(&s_buf, c);
        if (c == '_')
          state = STATE_UNDERSCORE;
        else
          state = STATE_KW_IDENTIFIER;
      }

      /* --- NUMBERS AND DOT --- */
      else if (isdigit(c) || c == '.') {
        at_line_start = false;
        scanner_buffer_append(&s_buf, c);
        if (c == '.')
          state = STATE_DOT_CHECK;
        else if (c == '0')
          state = STATE_ZERO;
        else
          state = STATE_INT;
      }

      /* --- STRINGS --- */
      else if (c == '"') {
        at_line_start = false;
        state = STATE_QUOTE_1;
      }

      /* --- Operators --- */
      else if (c == '=') {
        at_line_start = false;
        state = STATE_ASSIGN;
      } else if (c == '+') {
        at_line_start = false;
        state = STATE_PLUS;
      } else if (c == '-') {
        at_line_start = false;
        state = STATE_MINUS;
      } else if (c == '*') {
        at_line_start = false;
        state = STATE_MULTIPLY;
      } else if (c == '>') {
        at_line_start = false;
        state = STATE_GREATER_THAN;
      } else if (c == '<') {
        at_line_start = false;
        state = STATE_LESS_THAN;
      } else if (c == '!') {
        at_line_start = false;
        state = STATE_NOT;
      } else if (c == '/') {
        state = STATE_SLASH;
      }

      /* --- PUNCTUATION AND BRACKETS --- */
      else if (c == ';') {
        at_line_start = false;
        state = STATE_SEMICOLON;
      } else if (c == ':') {
        at_line_start = false;
        state = STATE_COLON;
      } else if (c == ',') {
        at_line_start = false;
        state = STATE_COMMA;
      } else if (c == '(') {
        at_line_start = false;
        bracket_depth++;
        state = STATE_LPAR;
      } else if (c == ')') {
        at_line_start = false;
        if (bracket_depth > 0)
          bracket_depth--;
        state = STATE_RPAR;
      }

      /* --- ANNOTATIONS --- */
      else if (c == '@') {
        at_line_start = false;
        state = STATE_ANNOT;
      }

      /* --- EOF ---*/
      else if (c == EOF) {
        at_line_start = false;
        state = STATE_EOF;
      }

      else
        return ERR_LEXICAL;

      break;
    }
  }
}
