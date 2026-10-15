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

// ---------- Section 7.4.3 from spec

/**
 * @brief Prints ADD (dest = source1 + source2)
 *
 * @param dest Destination variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int/float), jinak (error 53)
 */
void codegen_add(const char *dest, const char *source1, const char *source2);

/**
 * @brief Prints SUB (dest = source1 - source2)
 *
 * @param dest Destination variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int/float), jinak (error 53)
 */
void codegen_sub(const char *dest, const char *source1, const char *source2);

/**
 * @brief Prints MUL (dest = source1 * source2)
 *
 * @param dest Destination variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int/float), jinak (error 53)
 */
void codegen_mul(const char *dest, const char *source1, const char *source2);

/**
 * @brief Prints DIV (dest = source1 / source2)
 *
 * @param dest Destination variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu float, jinak (error 53)
 * @note Deleni nulou (error 57)
 */
void codegen_div(const char *dest, const char *source1, const char *source2);

/**
 * @brief Prints IDIV (dest = source1 / source2)
 *
 * @param dest Destination variable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu int, jinak (error 53)
 * @note Deleni nulou (error 57)
 * @note IDIV zaokrouhluje k -nekonecnu (IFJ26 k nule)
 */
void codegen_idiv(const char *dest, const char *source1, const char *source2);
#endif
