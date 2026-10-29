/**
 * @file builtins.h
 * @brief Code generation for IFJ26 built-in functions (interface)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#ifndef IFJ_IAL_PROJECT_BUILTINS_H
#define IFJ_IAL_PROJECT_BUILTINS_H

/**
 * @brief Temporary type of a value passed to buildin functions
 *
 * @note Nahradit spolecnym DataType
 */
typedef enum {
  BUILTIN_TYPE_INT,
  BUILTIN_TYPE_DOUBLE,
  BUILTIN_TYPE_STRING,
  BUILTIN_TYPE_UNIT,
} BuiltinType;

/**
 * @brief Generates print(value)
 *
 * @param value Operand to print (variable or constant)
 * @param type Type of the value
 */
void builtin_print(const char *value, BuiltinType type);

/**
 * @brief Generates readLine()
 *
 * @param result Variable that receives the string
 *
 * @note Vraci radek vcetne konce radku, pri EOF prazdny retezec
 */
void builtin_readline(const char *result);

/**
 * @brief Generates readInt()
 *
 * @param result Variable that receives the int
 */
void builtin_readint(const char *result);

/**
 * @brief Generates readDouble()
 *
 * @param result Variable that receives the float
 */
void builtin_readdouble(const char *result);

/**
 * @brief Generates toStr(source)
 *
 * @param result Variable that receives the string
 * @param source Value to convert
 * @param type Type of the value
 */
void builtin_tostr(const char *result, const char *source, BuiltinType type);
#endif