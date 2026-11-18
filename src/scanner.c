/**
 * @file scanner.c
 * @brief Lexical analyzer implementation for the IFJ26 compiler
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Filip Haza (xhazafi00)
 */
#define _POSIX_C_SOURCE 200809L

#include "scanner.h"
#include "errors.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_INIT_CAPACITY 32

static FILE *source_file = NULL;
static size_t current_line = 1;
static size_t space_count = 0;
static int bracket_depth = 0;
static int comment_depth = 0;
static bool at_line_start = true;
static bool has_tab = false;
static bool com_newline = false;
static bool line_had_token = false;
static bool indent_locked = false;

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

// ---------- indentation stack struct
typedef struct {
  size_t indentation[128];
  int top;
} indent_stack_t;

static indent_stack_t s_stack;
static size_t pending_dedents;

static void init_stack(indent_stack_t *s_stack) {
  s_stack->top = 0;
  s_stack->indentation[0] = 0;
}

static void push(indent_stack_t *s_stack, size_t level) {
  if (s_stack->top + 1 < 128) {
    s_stack->top++;
    s_stack->indentation[s_stack->top] = level;
  } else {
    exit(ERR_INTERNAL);
  }
}

static void pop(indent_stack_t *s_stack) {
  if (s_stack->top > 0) {
    s_stack->top--;
  }
}
static size_t top_peek(indent_stack_t *s_stack) {
  return (s_stack->indentation[s_stack->top]);
}

// ---------- keyword struct
typedef struct {
  const char *keyword;
  TokenType type;
} keyword_entry_t;

static const keyword_entry_t KEYWORDS[] = {
    {"def", TOKEN_KW_DEF},         {"do", TOKEN_KW_DO},
    {"Double", TOKEN_KW_DOUBLE},   {"else", TOKEN_KW_ELSE},
    {"if", TOKEN_KW_IF},           {"import", TOKEN_KW_IMPORT},
    {"Int", TOKEN_KW_INT},         {"return", TOKEN_KW_RETURN},
    {"String", TOKEN_KW_STRING},   {"then", TOKEN_KW_THEN},
    {"Unit", TOKEN_KW_UNIT},       {"val", TOKEN_KW_VAL},
    {"var", TOKEN_KW_VAR},         {"while", TOKEN_KW_WHILE},
    {"true", TOKEN_KW_TRUE},       {"false", TOKEN_KW_FALSE},
    {"Boolean", TOKEN_KW_BOOLEAN}, {"to", TOKEN_KW_TO},
    {"until", TOKEN_KW_UNTIL},     {"by", TOKEN_KW_BY},
    {"for", TOKEN_KW_FOR},
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
  STATE_QUOTE_1,          // Wait: 1st '"'
  STATE_STRING,           // In string literal
  STATE_STRING_ESCAPE,    // Wait: '\' escape
  STATE_QUOTE_2,          // Wait/Final: 2nd '"'
  STATE_MSTR,             // In multi-line (""")
  STATE_MSTR_END_1,       // Wait: 1st '"' of """
  STATE_MSTR_END_2,       // Wait: 2nd '"' of """
  STATE_MSTR_EXTRA_QUOTE, // extra quotes are part of string

  /* --- Operators and Delimiters --- */
  STATE_ASSIGN,       // Decision for '=' or '=='
  STATE_LESS_THAN,    // Decision for '<' or '<='
  STATE_GREATER_THAN, // Decision for '>' or '>='

  STATE_PLUS,     // Final: '+'
  STATE_MINUS,    // Final: '-'
  STATE_MULTIPLY, // Final: '*'

  STATE_NOT, // Decision for '!' / '!='

  STATE_AND, // Logical operand &&
  STATE_OR,  // Logical operand ||

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

static void scanner_static_cleanup(void) {
  bracket_depth = 0;
  comment_depth = 0;
  space_count = 0;
  pending_dedents = 0;
  at_line_start = true;
  has_tab = false;
  com_newline = false;
  line_had_token = false;
  indent_locked = false;
}

void scanner_init(FILE *source) {
  scanner_buffer_free(&s_buf);

  if (source != NULL) {
    source_file = source;
  } else {
    source_file = stdin;
  }
  current_line = 1;

  scanner_static_cleanup();

  scanner_buffer_init(&s_buf);
  init_stack(&s_stack);
}

void scanner_cleanup(void) {
  scanner_buffer_free(&s_buf);
  source_file = NULL;

  scanner_static_cleanup();
}

int scanner_get_next_token(Token *token) {

  // check if token is passed right from parser
  if (token == NULL) {
    fprintf(stderr, "Internal error: Null pointer passed as token.\n");
    return (ERR_INTERNAL);
  }

  memset(&token->attr, 0, sizeof(token->attr));

  // pending dedents have priority
  if (pending_dedents > 0) {
    pending_dedents--;
    token->type = TOKEN_DEDENT;
    token->line = current_line;
    return 0;
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
      if (s_stack.top > 0) {
        pop(&s_stack);
        token->type = TOKEN_DEDENT;
        token->line = current_line;
        return 0;
      }
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
      } else if (c == '-') { // control extension
        token->type = TOKEN_ARROW;
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
        ungetc(c, source_file); // Boolthen extension
        token->type = TOKEN_NOT;
        token->line = current_line;
        return 0;
      }

    case STATE_AND:
      if (c == '&') {
        token->type = TOKEN_AND;
        token->line = current_line;
        return 0;
      } else {
        return ERR_LEXICAL;
      }

    case STATE_OR:
      if (c == '|') {
        token->type = TOKEN_OR;
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
        //@main cannot be followed by alnum or _
        int next_c = fgetc(source_file);
        if (isalnum(next_c) || next_c == '_') {
          return ERR_LEXICAL;
        }
        if (next_c != EOF) {
          ungetc(next_c, source_file);
        }

        token->type = TOKEN_ANNOT_MAIN;
        token->line = current_line;
        return 0;
      } else {
        return ERR_LEXICAL;
      }

    /* --- DIVISION AND COMMENTS --- */
    case STATE_SLASH:
      if (com_newline) {
        return ERR_LEXICAL;
      }
      if (at_line_start && has_tab) {
        return ERR_LEXICAL;
      }
      at_line_start = false;
      ungetc(c, source_file);
      token->type = TOKEN_DIV;
      token->line = current_line;
      return 0;

    case STATE_COM_LINE:
      if (c == EOF) {
        if (at_line_start || bracket_depth > 0) {
          state = STATE_EOF;
          break;
        }
        at_line_start = true;
        token->type = TOKEN_EOL;
        token->line = current_line;
        return 0;
      } else if (c == '\n') {
        current_line++;
        com_newline = false;
        indent_locked = false;

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
        if (bracket_depth <= 0) {
          com_newline = true;
          at_line_start = true;
          space_count = 0;
          has_tab = false;
          indent_locked = false;
        }
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
        if (bracket_depth <= 0) {
          com_newline = true;
          at_line_start = true;
          space_count = 0;
          has_tab = false;
          indent_locked = false;
        }
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
          if (com_newline) {
            if (line_had_token && bracket_depth <= 0) {
              line_had_token = false;
              at_line_start = true;
              token->type = TOKEN_EOL;
              token->line = current_line;
              return 0;
            }
          } else {
            if (at_line_start) {
              indent_locked = true;
            }
          }
          state = STATE_START;
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
        if (bracket_depth <= 0) {
          com_newline = true;
          at_line_start = true;
          space_count = 0;
          has_tab = false;
          indent_locked = false;
        }
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
        if (!token->attr.str_val) {
          exit(ERR_INTERNAL);
        }
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
      } else if (isdigit(c) || c == '_' || isalpha(c)) {
        // ERROR: number cannot begin with 0 or be followed by letter/underscore
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
      } else if (isalpha(c)) {
        // ERROR: letter directly after integer (e.g. 123abc)
        return ERR_LEXICAL;
      } else {
        // Return int token
        ungetc(c, source_file);
        errno = 0;
        long long val = strtoll(s_buf.data, NULL, 10);
        if (errno == ERANGE) {
          return ERR_LEXICAL;
        }
        token->attr.int_val = val;
        token->type = TOKEN_INT_LIT;
        token->line = current_line;
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
      } else if (isalpha(c)) {
        // ERROR: letter directly after double (e.g. 1.5abc)
        return ERR_LEXICAL;
      } else {
        // Return double token
        ungetc(c, source_file);
        double val = strtod(s_buf.data, NULL);
        if (isinf(val)) {
          return ERR_LEXICAL;
        }
        token->type = TOKEN_DOUBLE_LIT;
        token->line = current_line;
        token->attr.double_val = val;
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
      } else if (isalpha(c)) {
        // ERROR: letter directly after exponent (e.g. 1e5xyz)
        return ERR_LEXICAL;
      } else {
        ungetc(c, source_file);
        double val = strtod(s_buf.data, NULL);
        if (isinf(val)) {
          return ERR_LEXICAL;
        }
        token->type = TOKEN_DOUBLE_LIT;
        token->line = current_line;
        token->attr.double_val = val;
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
      if (isalnum(c) || c == '_') {
        // ERROR: alphanumeric/underscore immediately following 'd'/'D' suffix
        // (e.g. 1.5dx)
        return ERR_LEXICAL;
      }
      ungetc(c, source_file);
      double val = strtod(s_buf.data, NULL);
      if (isinf(val)) {
        return ERR_LEXICAL;
      }
      token->type = TOKEN_DOUBLE_LIT;
      token->line = current_line;
      token->attr.double_val = val;
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
      } else if (c == EOF || (unsigned char)c < 32 || (unsigned char)c > 127) {
        // unterminated string literal and not allowed chars
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
      } else if (c == EOF || (unsigned char)c < 32 || (unsigned char)c > 127) {
        // unterminated string literal and not allowed chars
        return ERR_LEXICAL;
      } else if (c == '"') {
        token->type = TOKEN_STRING_LIT;
        token->line = current_line;
        token->attr.str_val = strdup(s_buf.data);
        if (!token->attr.str_val) {
          exit(ERR_INTERNAL);
        }
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
        if (!token->attr.str_val) {
          exit(ERR_INTERNAL);
        }
        return 0;
      }

    case STATE_MSTR:
      if (c == EOF) {
        // unterminated string literal
        return ERR_LEXICAL;
      } else if (c == '"') {
        // string ending quote 1
        state = STATE_MSTR_END_1;
        break;
      } else if (c == '\n') {
        current_line++;
        scanner_buffer_append(&s_buf, c);
        break;
      } else if ((unsigned char)c > 127 ||
                 ((unsigned char)c < 32 && c != '\r' && c != '\t')) {
        // not allowed characters
        return ERR_LEXICAL;
      } else {
        // string body
        scanner_buffer_append(&s_buf, c);
        break;
      }

    case STATE_MSTR_END_1:
      if (c == EOF) {
        // unterminated string literal
        return ERR_LEXICAL;
      } else if (c == '"') {
        // string ending quote 2
        state = STATE_MSTR_END_2;
        break;
      } else if (c == '\n') {
        current_line++;
        scanner_buffer_append(&s_buf, '"');
        scanner_buffer_append(&s_buf, c);
        state = STATE_MSTR;
        break;
      } else if ((unsigned char)c > 127 ||
                 ((unsigned char)c < 32 && c != '\r' && c != '\t')) {
        // not allowed characters
        return ERR_LEXICAL;
      } else {
        // string body
        scanner_buffer_append(&s_buf, '"');
        scanner_buffer_append(&s_buf, c);
        state = STATE_MSTR;
        break;
      }

    case STATE_MSTR_END_2:
      if (c == EOF) {
        // unterminated string literal
        return ERR_LEXICAL;
      } else if (c == '"') {
        // check for extra quotes
        state = STATE_MSTR_EXTRA_QUOTE;
        break;

      } else if (c == '\n') {
        current_line++;
        scanner_buffer_append(&s_buf, '"');
        scanner_buffer_append(&s_buf, '"');
        scanner_buffer_append(&s_buf, c);
        state = STATE_MSTR;
        break;
      } else if ((unsigned char)c > 127 ||
                 ((unsigned char)c < 32 && c != '\r' && c != '\t')) {
        // not allowed characters
        return ERR_LEXICAL;
      } else {
        // string body
        scanner_buffer_append(&s_buf, '"');
        scanner_buffer_append(&s_buf, '"');
        scanner_buffer_append(&s_buf, c);
        state = STATE_MSTR;
        break;
      }
    case STATE_MSTR_EXTRA_QUOTE:
      if (c == '"') {
        // extra quote
        scanner_buffer_append(&s_buf, '"');
        break;
      } else {
        // string end
        ungetc(c, source_file);
        token->type = TOKEN_STRING_LIT;
        token->line = current_line;
        token->attr.str_val = strdup(s_buf.data);
        if (!token->attr.str_val) {
          exit(ERR_INTERNAL);
        }
        return 0;
      }

    case STATE_START:

      /* --- WHITESPACE --- */
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        /* --- LF --- */
        if (c == '\n') {
          current_line++;
          com_newline = false;
          indent_locked = false;

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
            if (!indent_locked) {
              space_count++;
            }
            break;
          } else if (c == '\t') {
            if (!indent_locked) {
              has_tab = true;
            }
            break;
          } else {
            break;
          }
        } else {
          break;
        }
      }

      /* --- EOF ---*/
      if (c == EOF) {
        if (at_line_start || bracket_depth > 0) {
          state = STATE_EOF;
          break;
        }
        at_line_start = true;
        token->type = TOKEN_EOL;
        token->line = current_line;
        return 0;
      }

      if (com_newline && c != '/') {
        return ERR_LEXICAL;
      }

      if (c == '/') {
        int next = fgetc(source_file);
        if (next == '/') {
          state = STATE_COM_LINE;
          break;
        } else if (next == '*') {
          if (comment_depth == 0) {
            line_had_token = !at_line_start;
          }
          comment_depth++;
          state = STATE_COM_BLOCK;
          break;
        }
        if (next != EOF) {
          ungetc(next, source_file);
        }
      }

      if (com_newline) {
        return ERR_LEXICAL;
      }

      /*--- INDENTATION CONTROL ---*/
      if (at_line_start && bracket_depth <= 0) {
        if (has_tab) {
          return ERR_LEXICAL;
        }

        size_t current_indent = top_peek(&s_stack);

        // indent
        if (space_count > current_indent) {
          push(&s_stack, space_count);
          ungetc(c, source_file);
          at_line_start = false;
          token->type = TOKEN_INDENT;
          token->line = current_line;
          return 0;
        }
        // dedent
        else if (space_count < current_indent) {
          while (s_stack.top > 0 && space_count < top_peek(&s_stack)) {
            pop(&s_stack);
            pending_dedents++;
          }
          // wrong indentation
          if (space_count != top_peek(&s_stack)) {
            return ERR_LEXICAL;
          }
          // return first dedent
          pending_dedents--;
          ungetc(c, source_file);
          at_line_start = false;
          token->type = TOKEN_DEDENT;
          token->line = current_line;
          return 0;
        }
        // same indentation
        at_line_start = false;
      } else if (at_line_start && bracket_depth > 0) {
        // inside brackets, indentation is ignored
        at_line_start = false;
      }

      /* --- IDENTIFIERS AND KEYWORDS --- */
      if (isalpha(c) || c == '_') {
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
      } else if (c == '/') {
        at_line_start = false;
        state = STATE_SLASH;
      }

      /* --- Logical operators --- */
      else if (c == '!') {
        at_line_start = false;
        state = STATE_NOT;
      } else if (c == '&') {
        at_line_start = false;
        state = STATE_AND;
      } else if (c == '|') {
        at_line_start = false;
        state = STATE_OR;
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

      else
        return ERR_LEXICAL;

      break;
    }
  }
}
