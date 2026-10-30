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
  BUILTIN_TYPE_UNIT
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

/**
 * @brief Generates toInt(value)
 *
 * @param result Variable that receives the int value
 * @param value Double operand
 */
void builtin_toint(const char *result, const char *value);

/**
 * @brief Generates strLength(str)
 *
 * @param result Variable that receives the length (int)
 * @param str String operand
 */
void builtin_strlength(const char *result, const char *str);

/**
 * @brief Generates definition of the function
 */
void builtin_substr_definition(void);

/**
 * @brief Generates a call of subStr(value, start, end)
 *
 * @param result Variable that receives the substring
 * @param value Source string
 * @param start Start index
 * @param end End index
 */
void builtin_substr_call(const char *result, const char *value,
                         const char *start, const char *end);

/**
 * @brief Generates a definition of the function
 */
void builtin_ord_definition(void);

/**
 * @brief Generates a call of subStr(value, start, end)
 *
 * @param result Variable that receives the ASCII code
 * @param string Source string
 * @param index Index of the character
 */
void builtin_ord_call(const char *result, const char *string,
                      const char *index);

/**
 * @brief Generates chr(value)
 *
 * @param result Variable that receives the one character string
 * @param value Integer ASCII code
 */
void builtin_chr(const char *result, const char *value);
#endif