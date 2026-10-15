/**
 * @file codegen.c
 * @brief IFJcode26 target code generator (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#include "codegen.h"
#include <stdio.h>

void codegen_header(void) { printf(".IFJcode26\n"); }

// ---------- Section 7.4.1 from spec
void codegen_defvar(const char *var) { printf("DEFVAR %s\n", var); }

void codegen_move(const char *dest, const char *source) {
  printf("MOVE %s %s\n", dest, source);
}

// ---------- Section 7.4.3 from spec
void codegen_add(const char *dest, const char *source1, const char *source2) {
  printf("ADD %s %s %s\n", dest, source1, source2);
}

void codegen_sub(const char *dest, const char *source1, const char *source2) {
  printf("SUB %s %s %s\n", dest, source1, source2);
}

void codegen_mul(const char *dest, const char *source1, const char *source2) {
  printf("MUL %s %s %s\n", dest, source1, source2);
}

void codegen_div(const char *dest, const char *source1, const char *source2) {
  printf("DIV %s %s %s\n", dest, source1, source2);
}

void codegen_idiv(const char *dest, const char *source1, const char *source2) {
  printf("IDIV %s %s %s\n", dest, source1, source2);
}