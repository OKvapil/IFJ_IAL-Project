/**
 * @file builtins.c
 * @brief Code generation for IFJ26 built-in functions (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#include "builtins.h"
#include "codegen.h"
#include <stdbool.h>

void builtin_print(const char *value, BuiltinType type) {
  if (type == BUILTIN_TYPE_UNIT) {
    codegen_write("string@()");
  } else {
    codegen_write(value);
  }
}

void builtin_readline(const char *result) { codegen_read(result, "string"); }

void builtin_readint(const char *result) { codegen_read(result, "int"); }

void builtin_readdouble(const char *result) { codegen_read(result, "float"); }

void builtin_tostr(const char *result, const char *source, BuiltinType type) {
  switch (type) {
  case BUILTIN_TYPE_INT:
    codegen_int2str(result, source);
    break;
  case BUILTIN_TYPE_DOUBLE:
    codegen_float2str(result, source);
    break;
  case BUILTIN_TYPE_STRING:
    codegen_move(result, source);
    break;
  case BUILTIN_TYPE_UNIT:
    codegen_move(result, "()");
    break;
  }
}
