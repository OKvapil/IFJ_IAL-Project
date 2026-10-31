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
#define BUFFER_INIT_CAPACITY 32

static FILE *source_file = NULL;
static size_t current_line = 1;

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
  STATE_DOT,               // Final: standalone dot

  /* --- Strings --- */
  STATE_QUOTE_1,       // Wait: 1st '"'
  STATE_STRING,        // In string literal
  STATE_STRING_ESCAPE, // Wait: '\' escape
  STATE_STRING_END,    // Final: closed string
  STATE_QUOTE_2,       // Wait/Final: 2nd '"'
  STATE_MSTR,          // In multi-line (""")
  STATE_MSTR_END_1,    // Wait: 1st '"' of """
  STATE_MSTR_END_2,    // Wait: 2nd '"' of """
  STATE_MSTR_END_3,    // Final: closed """

} scanner_state_t;

void scanner_init(FILE *source) {
  if (source != NULL) {
    source_file = source;
  } else {
    source_file = stdin;
  }
  current_line = 1;
  scanner_buffer_init(&s_buf);
}

void scanner_cleanup(void) {
  scanner_buffer_free(&s_buf);
  source_file = NULL;
}

int scanner_get_next_token(Token *token) {

  // check if token is passed right from parser
  if (token == NULL) {
    fprintf(stderr, "Internal error: Null pointer passed as token.\n");
    return (ERR_INTERNAL);
  }

  // cleanup before next token
  scanner_buffer_clear(&s_buf);

  token->line = current_line;
}
