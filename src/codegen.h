/**
 * @file codegen.h
 * @brief IFJcode26 target code generator (generator)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#ifndef IFJ_IAL_PROJECT_CODEGEN_H
#define IFJ_IAL_PROJECT_CODEGEN_H

/**
 * @brief Prints required "IFJcode26" header line
 */
void codegen_header(void);

// ---------- Section 7.4.1 from spec

/**
 * @brief Prints DEFVAR (define a new variable)
 *
 * @param var Variable operand including frame "LF@var"
 *
 * @note Stejna promenna nesmi byt v jednom ramci definovana dvakrat (error 52).
 * @note Cteni promenne pred prirazenim (error 56), pouziti nedefinovane
 * promenne (error 54).
 */
void codegen_defvar(const char *var);

/**
 * @brief Prints MOVE (copies value into var)
 *
 * @param dest Destination variable must be defined already
 * @param source Variable or constant to copy
 *
 * @note Cilova promenna musi byt predem definovana (error 54)
 */
void codegen_move(const char *dest, const char *source);

// ---------- Section 7.4.2 from spec

// ---------- Section 7.4.3 from spec

/**
 * @brief Prints ADD (result = source1 + source2)
 *
 * @param result Result variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int/float), jinak (error 53)
 */
void codegen_add(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints SUB (result = source1 - source2)
 *
 * @param result Result variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int/float), jinak (error 53)
 */
void codegen_sub(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints MUL (result = source1 * source2)
 *
 * @param result Result variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int/float), jinak (error 53)
 */
void codegen_mul(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints DIV (result = source1 / source2)
 *
 * @param result Result variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu float, jinak (error 53)
 * @note Deleni nulou (error 57)
 */
void codegen_div(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints IDIV (result = source1 / source2)
 *
 * @param result Result variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu int, jinak (error 53)
 * @note Deleni nulou (error 57)
 * @note IDIV zaokrouhluje k -nekonecnu (IFJ26 k nule)
 */
void codegen_idiv(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints LT (result = source1 < source2)
 *
 * @param result Result variable (recives bool)
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int, float, bool, string), jinak
 * (error 53)
 * @note Lexikograficke porovnani false < true
 */
void codegen_lt(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints GT (result = source1 > source2)
 *
 * @param result Result variable (bool)
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int, float, bool, string), jinak
 * (error 53)
 * @note Lexikograficke porovnani false < true
 */
void codegen_gt(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints EQ (result = source1 == source2)
 *
 * @param result Result variable (bool)
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int, float, bool, string), jinak
 * (error 53)
 * @note Lexikograficke porovnani false < true
 */
void codegen_eq(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints AND (result = source1 AND source2)
 *
 * @param result Result variable (bool)
 * @param source1 First operand (bool)
 * @param source2 Second operand (bool)
 *
 * @note Oba operandy musi byt typu bool, jinak (error 53)
 */
void codegen_and(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints OR (result = source1 OR source2)
 *
 * @param result Result variable (bool)
 * @param source1 First operand (bool)
 * @param source2 Second operand (bool)
 *
 * @note Oba operandy musi byt typu bool, jinak (error 53)
 */
void codegen_or(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints NOT (result = NOT source)
 *
 * @param result Result variable (bool)
 * @param source Operand (bool)
 *
 * @note Operand musi byt typu bool, jinak (error 53)
 */
void codegen_not(const char *result, const char *source);

/**
 * @brief Prints INT2FLOAT (converts a int value into float value )
 *
 * @param result Result variable that recives float value
 * @param source Integer value (variable or constant)
 *
 * @note Operand musi byt typu int, jinak (error 53)
 */
void codegen_int2float(const char *result, const char *source);

/**
 * @brief Prints FLOAT2INT (converts a float value into int value )
 *
 * @param result Result variable that recives int value
 * @param source Integer value (variable or constant)
 *
 * @note Operand musi byt typu float, jinak (error 53)
 */
void codegen_float2int(const char *result, const char *source);

/**
 * @brief Prints INT2CHAR (converts ASCII code to a one character string)
 *
 * @param result Result variable that recives the string
 * @param source Integer ASCII code
 *
 * @note Hodnota mimo interval 0-127 (error 58)
 * @note Operand musi byt typu int, jinak (error 53)
 */
void codegen_int2char(const char *result, const char *source);

/**
 * @brief Prints STRI2INT (result = ASCII code of str[position])
 *
 * @param result Result variable that recives ASCII code (int)
 * @param str Source string (variable or constant)
 * @param position Index of character counted from 0 (int)
 *
 * @note Operandy musi byt typu string a int v tomto poradi, jinak (error 53)
 * @note Index mimo retezec (error 58)
 */
void codegen_stri2int(const char *result, const char *str,
                      const char *position);

/**
 * @brief Prints FLOAT2STR (converts a float value to a string)
 *
 * @param result Result variable that recives string
 * @param source Float value (variable or constant)
 *
 * @note Operand musi byt typu float, jinak (error 53)
 * @note Format odpovida vestavene funkci toStr z IFJ26
 */
void codegen_float2str(const char *result, const char *source);

/**
 * @brief Prints INT2STR (converts a int value to a string)
 *
 * @param result Result variable that recives string
 * @param source Integer value (variable or constant)
 *
 * @note Operand musi byt typu int, jinak (error 53)
 */
void codegen_int2str(const char *result, const char *source);
// ---------- Section 7.4.4 from spec

// ---------- Section 7.4.5 from spec

// ---------- Section 7.4.6 from spec

// ---------- Section 7.4.7 from spec

// ---------- Section 7.4.8 from spec

// ---------- Section 7.4.9 from spec
#endif
