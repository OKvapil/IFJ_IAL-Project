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
 * odpovídá zadání a struktuře jazyka ifj26. Pouzivej anglické kometnáře, snake_case. 
 * 
 * Pokud ti neco není jasné doptávej se."
 * ============================================================================ */

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
        fprintf(stderr, "Internal error: Memory allocation failed for scanner buffer.\n");
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
            fprintf(stderr, "Internal error: Memory reallocation failed for scanner buffer.\n");
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