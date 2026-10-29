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

void builtin_toint(const char *result, const char *value) {
  codegen_float2int(result, value);
}

void builtin_strlength(const char *result, const char *str) {
  codegen_strlen(result, str);
}

void builtin_substr_definition(void) {
  codegen_label("$builtin_substr");
  codegen_createframe();
  codegen_pushframe();
  codegen_defvar("LF@value");
  codegen_defvar("LF@start");
  codegen_defvar("LF@end");
  codegen_defvar("LF@len");
  codegen_defvar("LF@cond");
  codegen_defvar("LF@cond_second");
  codegen_defvar("LF@result");
  codegen_defvar("LF@index");
  codegen_defvar("LF@char");
  codegen_pops("LF@end");
  codegen_pops("LF@start");
  codegen_pops("LF@value");
  codegen_strlen("LF@len", "LF@value");

  // start < 0
  codegen_lt("LF@cond", "LF@start", "int@0");
  codegen_jumpifeq("$builtin_substr_error", "LF@cond", "bool@true");

  // end < 0
  codegen_lt("LF@cond", "LF@end", "int@0");
  codegen_jumpifeq("$builtin_substr_error", "LF@cond", "bool@true");

  // start > end
  codegen_gt("LF@cond", "LF@start", "LF@end");
  codegen_jumpifeq("$builtin_substr_error", "LF@cond", "bool@true");

  // start >= len
  codegen_gt("LF@cond", "LF@start", "LF@len");
  codegen_eq("LF@cond_second", "LF@start", "LF@len");
  codegen_or("LF@cond", "LF@cond", "LF@cond_second");
  codegen_jumpifeq("$builtin_substr_error", "LF@cond", "bool@true");

  // end > len
  codegen_gt("LF@cond", "LF@end", "LF@len");
  codegen_jumpifeq("$builtin_substr_error", "LF@cond", "bool@true");

  // loop: result = value[start---end)
  codegen_move("LF@result", "string@");
  codegen_move("LF@index", "LF@start");
  codegen_label("$builtin_substr_loop");
  codegen_jumpifeq("$builtin_substr_end", "LF@index", "LF@end");
  codegen_getchar("LF@char", "LF@value", "LF@index");
  codegen_concat("LF@result", "LF@result", "LF@char");
  codegen_add("LF@index", "LF@index", "int@1");
  codegen_jump("$builtin_substr_loop");
  codegen_label("$builtin_substr_end");
  codegen_pushs("LF@result");
  codegen_popframe();
  codegen_return();

  codegen_label("$builtin_substr_error");
  codegen_exit("int@40");
}

void builtin_substr_call(const char *result, const char *value,
                         const char *start, const char *end) {
  codegen_pushs(value);
  codegen_pushs(start);
  codegen_pushs(end);
  codegen_call("$builtin_substr");
  codegen_pops(result);
}
