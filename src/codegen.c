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

void codegen_createframe(void) { printf("CREATEFRAME\n"); }

void codegen_pushframe(void) { printf("PUSHFRAME\n"); }

void codegen_popframe(void) { printf("POPFRAME\n"); }

void codegen_return(void) { printf("RETURN\n"); }

void codegen_call(const char *label) { printf("CALL %s\n", label); }
// ---------- Section 7.4.2 from spec
void codegen_pushs(const char *source) { printf("PUSHS %s\n", source); }

void codegen_pops(const char *result) { printf("POPS %s\n", result); }

void codegen_clears(void) { printf("CLEARS\n"); }

// ---------- Section 7.4.3 from spec
void codegen_add(const char *result, const char *source1, const char *source2) {
  printf("ADD %s %s %s\n", result, source1, source2);
}

void codegen_sub(const char *result, const char *source1, const char *source2) {
  printf("SUB %s %s %s\n", result, source1, source2);
}

void codegen_mul(const char *result, const char *source1, const char *source2) {
  printf("MUL %s %s %s\n", result, source1, source2);
}

void codegen_div(const char *result, const char *source1, const char *source2) {
  printf("DIV %s %s %s\n", result, source1, source2);
}

void codegen_idiv(const char *result, const char *source1,
                  const char *source2) {
  printf("IDIV %s %s %s\n", result, source1, source2);
}

void codegen_lt(const char *result, const char *source1, const char *source2) {
  printf("LT %s %s %s\n", result, source1, source2);
}

void codegen_gt(const char *result, const char *source1, const char *source2) {
  printf("GT %s %s %s\n", result, source1, source2);
}

void codegen_eq(const char *result, const char *source1, const char *source2) {
  printf("EQ %s %s %s\n", result, source1, source2);
}

void codegen_and(const char *result, const char *source1, const char *source2) {
  printf("AND %s %s %s\n", result, source1, source2);
}

void codegen_or(const char *result, const char *source1, const char *source2) {
  printf("OR %s %s %s\n", result, source1, source2);
}

void codegen_not(const char *result, const char *source) {
  printf("NOT %s %s\n", result, source);
}

void codegen_int2float(const char *result, const char *source) {
  printf("INT2FLOAT %s %s\n", result, source);
}

void codegen_float2int(const char *result, const char *source) {
  printf("FLOAT2INT %s %s\n", result, source);
}

void codegen_int2char(const char *result, const char *source) {
  printf("INT2CHAR %s %s\n", result, source);
}

void codegen_stri2int(const char *result, const char *str,
                      const char *position) {
  printf("STRI2INT %s %s %s\n", result, str, position);
}

void codegen_float2str(const char *result, const char *source) {
  printf("FLOAT2STR %s %s\n", result, source);
}

void codegen_int2str(const char *result, const char *source) {
  printf("INT2STR %s %s\n", result, source);
}

// ---------- Section 7.4.4 from spec
void codegen_read(const char *result, const char *type) {
  printf("READ %s %s\n", result, type);
}

void codegen_write(const char *symbol) { printf("WRITE %s\n", symbol); }

// ---------- Section 7.4.5 from spec
void codegen_concat(const char *result, const char *source1,
                    const char *source2) {
  printf("CONCAT %s %s %s\n", result, source1, source2);
}

void codegen_strlen(const char *result, const char *source) {
  printf("STRLEN %s %s\n", result, source);
}

void codegen_getchar(const char *result, const char *source,
                     const char *position) {
  printf("GETCHAR %s %s %s\n", result, source, position);
}

void codegen_setchar(const char *string, const char *position,
                     const char *source) {
  printf("SETCHAR %s %s %s\n", string, position, source);
}

// ---------- Section 7.4.6 from spec
void codegen_newarray(const char *result, const char *size) {
  printf("NEWARRAY %s %s\n", result, size);
}

void codegen_arrayget(const char *result, const char *array,
                      const char *position) {
  printf("SETCHAR %s %s %s\n", result, array, position);
}

void codegen_arrayset(const char *array, const char *position,
                      const char *element) {
  printf("ARRAYSET %s %s %s\n", array, position, element);
}
// ---------- Section 7.4.7 from spec
void codegen_type(const char *result, const char *source) {
  printf("TYPE %s %s\n", result, source);
}

void codegen_isint(const char *result, const char *source) {
  printf("ISINT %s %s\n", result, source);
}
// ---------- Section 7.4.8 from spec
void codegen_label(const char *label_literal) {
  printf("LABEL %s\n", label_literal);
}

void codegen_jump(const char *label) { printf("JUMP %s\n", label); }

void codegen_jumpifeq(const char *label, const char *source1,
                      const char *source2) {
  printf("JUMPIFEQ %s %s %s\n", label, source1, source2);
}

void codegen_jumpifneq(const char *label, const char *source1,
                       const char *source2) {
  printf("JUMPIFNEQ %s %s %s\n", label, source1, source2);
}

void codegen_exit(const char *result) { printf("EXIT %s\n", result); }
// ---------- Section 7.4.9 from spec
void codegen_break(void) { printf("BREAK\n"); }

void codegen_dprint(const char *symbol) { printf("DPRINT %s\n", symbol); }