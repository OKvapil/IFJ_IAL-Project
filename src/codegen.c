/**
 * @file codegen.c
 * @brief IFJcode26 target code generator (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#include "codegen.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// ---------- Section 7.4.3 stack instructions
void codegen_adds(void) { printf("ADDS\n"); }

void codegen_subs(void) { printf("SUBS\n"); }

void codegen_muls(void) { printf("MULS\n"); }

void codegen_divs(void) { printf("DIVS\n"); }

void codegen_idivs(void) { printf("IDIVS\n"); }

void codegen_lts(void) { printf("LTS\n"); }

void codegen_gts(void) { printf("GTS\n"); }

void codegen_eqs(void) { printf("EQS\n"); }

void codegen_ands(void) { printf("ANDS\n"); }

void codegen_ors(void) { printf("ORS\n"); }

void codegen_nots(void) { printf("NOTS\n"); }

void codegen_int2floats(void) { printf("INT2FLOATS\n"); }

void codegen_float2ints(void) { printf("FLOAT2INTS\n"); }

void codegen_int2chars(void) { printf("INT2CHARS\n"); }

void codegen_stri2ints(void) { printf("STRI2INTS\n"); }

void codegen_float2strs(void) { printf("FLOAT2STRS\n"); }

void codegen_int2strs(void) { printf("INT2STRS\n"); }
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
  printf("ARRAYGET %s %s %s\n", result, array, position);
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

// ---------- Section 7.4.7 stack instructions
void codegen_types(void) { printf("TYPES\n"); }

void codegen_isints(void) { printf("ISINTS\n"); }

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

void codegen_exit(const char *code) { printf("EXIT %s\n", code); }

// ---------- Section 7.4.8 stack instructions
void codegen_jumpifeqs(const char *label) { printf("JUMPIFEQS %s\n", label); }

void codegen_jumpifneqs(const char *label) { printf("JUMPIFNEQS %s\n", label); }

// ---------- Section 7.4.9 from spec
void codegen_break(void) { printf("BREAK\n"); }

void codegen_dprint(const char *symbol) { printf("DPRINT %s\n", symbol); }

// ---------- Unique label id generator
static int label_counter = 0;

int codegen_new_label_id(void) { return label_counter++; }

// ---------- String literal escaping
/**
 * @brief Checks whether a character must be escaped in an IFJcode26 string
 *
 * @param character Character to check
 *
 * @return true for characters with code 0 - 32, 35 and 92, false otherwise
 */
static bool needs_escape(unsigned char character) {
  return character == 92 || character == 35 || character <= 32;
}

void codegen_print_escaped_string(const char *string) {
  for (int idx = 0; string[idx] != '\0'; idx++) {
    unsigned char character = (unsigned char)string[idx];
    if (needs_escape(character)) {
      printf("\\%03d", character);
    } else {
      printf("%c", character);
    }
  }
}

// ---------- Constant formating
char *codegen_const_int(long long value) {
  int length = snprintf(NULL, 0, "int@%lld", value);
  if (length < 0) {
    return NULL;
  }

  char *result = malloc(length + 1);
  if (result == NULL) {
    return NULL;
  }

  snprintf(result, length + 1, "int@%lld", value);

  return result;
}

char *codegen_const_bool(bool value) {
  const char *text = value ? "bool@true" : "bool@false";

  char *result = malloc(strlen(text) + 1);
  if (result == NULL) {
    return NULL;
  }

  strcpy(result, text);
  return result;
}

char *codegen_const_float(double value) {
  int length = snprintf(NULL, 0, "float@%a", value);
  if (length < 0) {
    return NULL;
  }

  char *result = malloc(length + 1);
  if (result == NULL) {
    return NULL;
  }

  snprintf(result, length + 1, "float@%a", value);

  return result;
}

char *codegen_const_nil(void) {
  char *result = malloc(strlen("nil@nil") + 1);

  if (result == NULL) {
    return NULL;
  }
  strcpy(result, "nil@nil");
  return result;
}

char *codegen_const_string(const char *string) {
  unsigned int length = 7; // "string@"

  for (int idx = 0; string[idx] != '\0'; idx++) {
    length += needs_escape((unsigned char)string[idx]) ? 4 : 1;
  }

  char *result = malloc(length + 1);

  if (result == NULL) {
    return NULL;
  }

  strcpy(result, "string@");
  int position = strlen("string@");
  for (int idx = 0; string[idx] != '\0'; idx++) {
    unsigned char character = (unsigned char)string[idx];
    if (needs_escape(character)) {
      sprintf(result + position, "\\%03d", character);
      position += 4;
    } else {
      result[position] = (char)character;
      position++;
    }
  }
  result[position] = '\0';
  return result;
}