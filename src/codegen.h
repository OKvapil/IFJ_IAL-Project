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

#include <stdbool.h>


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

/**
 * @brief Prints CREATEFRAME (creates new temporary frame)
 *
 * @note Pripadny obsah TF se zahodi
 */
void codegen_createframe(void);

/**
 * @brief Prints PUSHFRAME (moves TF onto stack, it becomes LF)
 *
 * @note Po provedeni je TF nedefinovany, pred dalsim pouzitim se musi zavolat
 * CREATEFRAME
 * @note Pristup k nedefinovanemu ramci (chyba 55)
 */
void codegen_pushframe(void);

/**
 * @brief Prints POPFRAME (moves the top LF from stack to TF)
 *
 * @note Pristup k nedefinovanemu ramci (chyba 55)
 */
void codegen_popframe(void);

/**
 * @brief Prints RETURN (returns to position saved by CALL)
 *
 * @note Pri prazdnem zasobniku (error 56)
 */
void codegen_return(void);

/**
 * @brief Prints CALL (jumps to label and saves return position)
 *
 * @param label Label of called function "$factorial"
 *
 * @note Skok na neexistujici navesti (error 52)
 */
void codegen_call(const char *label);

// ---------- Section 7.4.2 from spec
/**
 * @brief Prints PUSHS (pushes value onto stack)
 *
 * @param source Value to push
 *
 * @note Pouziti nedefinovane promenne (error 54)
 * @note Cteni neinicializovane promenne (error 56)
 */
void codegen_pushs(const char *source);

/**
 * @brief Prints POPS (pops value from stack)
 *
 * @param result Variable that recieves popped value
 *
 * @note Pouziti nedefinovane promenne (error 54)
 * @note Pri prazdnem zasobniku (error 56)
 */
void codegen_pops(const char *result);

/**
 * @brief Prints CLEARS (clears the whole stack)
 */
void codegen_clears(void);
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
 */
void codegen_gt(const char *result, const char *source1, const char *source2);

/**
 * @brief Prints EQ (result = source1 == source2)
 *
 * @param result Result variable (bool)
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Oba operandy musi byt stejneho typu (int, float, bool, string) nebo
 * jeden z nich nil, jinak (error 53)
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
 * @param source Float value (variable or constant)
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

// ---------- Section 7.4.3 stack instructions
/**
 * @brief Prints ADDS (stack version of ADD)
 *
 * @note Vybere dve hodnoty ze zasobniku a ulozi jejich soucet
 * @note Stejna pravidla a chyby jako ADD
 */
void codegen_adds(void);

/**
 * @brief Prints SUBS (stack version of SUB)
 *
 * @note Vybere dve hodnoty ze zasobniku, ulozi 2. vybrany - 1. vybrany
 * @note Stejná pravidla a chyby jako SUB
 */
void codegen_subs(void);

/**
 * @brief Prints MULS (stack version of MUL)
 *
 * @note Vybere dve hodnoty ze zasobniku, ulozi 2. vybrany * 1. vybrany
 * @note Stejná pravidla a chyby jako MUL
 */
void codegen_muls(void);

/**
 * @brief Prints DIVS (stack version of DIV)
 *
 * @note Vybere dve hodnoty ze zasobniku, ulozi 2. vybrany/ 1. vybrany
 * @note Stejná pravidla a chyby jako DIV
 */
void codegen_divs(void);

/**
 * @brief Prints IDIVS (stack version of IDIV)
 *
 * @note Vybere dve hodnoty ze zasobniku, ulozi 2. vybrany/ 1. vybrany
 * @note Stejná pravidla a chyby jako IDIV
 */
void codegen_idivs(void);

/**
 * @brief Prints LTS (stack version of LT)
 *
 * @note Vybere dve hodnoty ze zasobniku, ulozi 2. vybrany < 1. vybrany (bool)
 * @note Stejná pravidla a chyby jako LT
 */
void codegen_lts(void);

/**
 * @brief Prints GTS (stack version of GT)
 *
 * @note Vybere dve hodnoty ze zasobniku, ulozi 2. vybrany > 1. vybrany (bool)
 * @note Stejná pravidla a chyby jako GT
 */
void codegen_gts(void);

/**
 * @brief Prints EQS (stack version of EQ)
 *
 * @note Vybere dve hodnoty ze zasobniku, ulozi 2. vybrany = 1. vybrany (bool)
 * @note Stejná pravidla a chyby jako EQ
 */
void codegen_eqs(void);

/**
 * @brief Prints ANDS (stack version of AND)
 *
 * @note Vybere dve hodnoty (bool) ze zasobniku a ulozi jejich konjunkci
 * @note Stejná pravidla a chyby jako AND
 */
void codegen_ands(void);

/**
 * @brief Prints ORS (stack version of OR)
 *
 * @note Vybere dve hodnoty (bool) ze zasobniku a ulozi jejich disjunkci
 * @note Stejna pravidla a chyby jako OR.
 */
void codegen_ors(void);

/**
 * @brief Prints NOTS (stack version of NOT)
 *
 * @note Vybere hodnotu (bool) ze zasobniku a ulozi jeji negaci
 * @note Stejna pravidla a chyby jako NOT.
 */
void codegen_nots(void);

/**
 * @brief Prints INT2FLOATS (stack version of INT2FLOAT)
 *
 * @note Vybere hodnotu typu int a ulozi ji prevedenou na float
 * @note Stejna pravidla a chyby jako INT2FLOAT.
 */
void codegen_int2floats(void);

/**
 * @brief Prints FLOAT2INTS (stack version of FLOAT2INT)
 *
 * @note Vybere hodnotu typu float a ulozi ji prevedenou na int
 * @note Stejna pravidla a chyby jako FLOAT2INT.
 */
void codegen_float2ints(void);

/**
 * @brief Prints INT2CHARS (stack version of INT2CHAR)
 *
 * @note Vybere hodnotu typu int a ulozi ji prevedenou na char
 * @note Stejna pravidla a chyby jako INT2CHAR.
 */
void codegen_int2chars(void);

/**
 * @brief Prints STRI2INTS (stack version of STRI2INTS)
 *
 * @note Vybere hodnotu typu string a ulozi ji prevedenou na int
 * @note Stejna pravidla a chyby jako STRI2INTS
 */
void codegen_stri2ints(void);

/**
 * @brief Prints FLOAT2STRS (stack version of FLOAT2STR)
 *
 * @note Vybere hodnotu typu float a ulozi ji prevedenou na string
 * @note Stejna pravidla a chyby jako FLOAT2STR
 */
void codegen_float2strs(void);

/**
 * @brief Prints INT2STRS (stack version of INT2STRS)
 *
 * @note Vybere hodnotu typu int a ulozi ji prevedenou na string
 * @note Stejna pravidla a chyby jako INT2STRS
 */
void codegen_int2strs(void);

// ---------- Section 7.4.4 from spec
/**
 * @brief Prints READ (reads value from sti)
 *
 * @param result Variable that recieves that value
 * @param type Type of the variable (int, bool, float, string)
 *
 * @note Typ se pise bez @ "int" ne "int@"
 */
void codegen_read(const char *result, const char *type);

/**
 * @brief Prints WRITE (writes a value to sto)
 *
 * @param symbol Value to print
 */
void codegen_write(const char *symbol);

// ---------- Section 7.4.5 from spec
/**
 * @brief Prints CONCAT (result = source1 + source2, string concatenation)
 *
 * @param result Variable that recieves concatenated string
 * @param source1 First string
 * @param source2 Second string
 *
 * @note Oba operandy musi byt typu string, jinak (error 53)
 */
void codegen_concat(const char *result, const char *source1,
                    const char *source2);

/**
 * @brief Prints STRLEN (result = length of the string)
 *
 * @param result Variable that recieves length of the sting
 * @param source String (variable or constant)
 *
 * @note Operand musi byt typu string, jinak (error 53)
 */
void codegen_strlen(const char *result, const char *source);

/**
 * @brief Prints GETCHAR (result = character of string[position])
 *
 * @param result Variable that recieves one character string
 * @param source Source string
 * @param position Index of the character
 *
 * @note Operandy musi byt typu string a int, jinak (error 53)
 * @note Index mimo retezec (error 58)
 */
void codegen_getchar(const char *result, const char *source,
                     const char *position);

/**
 * @brief Prints SETCHAR (replaces character at position in string)
 *
 * @param string Variable holding a string, modified in place
 * @param position Index of character replaced
 * @param source String whose first character is inserted
 *
 * @note Spravne typy operandu, jinak (error 53)
 * @note Index mimo retezec, nebo prazdny retezec (error 58)
 */
void codegen_setchar(const char *string, const char *position,
                     const char *source);

// ---------- Section 7.4.6 from spec
/**
 * @brief Prints NEWARRAY (creates an array)
 *
 * @param result Variable that receives array reference
 * @param size Number of elements
 *
 * @note Spatny typ operandu (error 53)
 */
void codegen_newarray(const char *result, const char *size);

/**
 * @brief Prints ARRAYGET (result = array[position])
 *
 * @param result Variable that receives the element
 * @param array Variable holding array reference
 * @param position Index of the element
 *
 * @note Spatny typ operandu (error 53)
 * @note Index mimo velikost pole (error 58)
 */
void codegen_arrayget(const char *result, const char *array,
                      const char *position);

/**
 * @brief Prints ARRAYSET (array[position] = element)
 *
 * @param array Variable holding array reference
 * @param position position of the element
 * @param element Element to store
 *
 * @note Spatny typ operandu (error 53)
 * @note Index mimo velikost pole (error 58)
 */
void codegen_arrayset(const char *array, const char *position,
                      const char *element);
// ---------- Section 7.4.7 from spec
/**
 * @brief Prints TYPE (result = int/bool/float/string/nil/array)
 *
 * @param result Variable that receives type name
 * @param source Variable or constant to inspect
 *
 * @note U neinicializovane promenne je vysledek prazdny retezec
 */
void codegen_type(const char *result, const char *source);

/**
 * @brief Prints ISINT (result = true if it is int or float ending with .00)
 *
 * @param result Variable that receives bool result
 * @param source Number to be tested
 *
 * @note Operand typu float nebo int, jinak (error 53)
 */
void codegen_isint(const char *result, const char *source);

// ---------- Section 7.4.7 stack instructions
/**
 * @brief Prints TYPES (stack version of TYPE)
 *
 * @note Vybere jednu hodnotu ze zasobniku a ulozi retezec s nazvem jejiho typu
 * @note Stejna pravidla jako TYPE.
 */
void codegen_types(void);

/**
 * @brief Prints ISINTS (stack version of ISINT)
 *
 * @note Vybere jednu hodnotu ze zasobniku a (bool) pokud je to int nebo float s
 * .00
 * @note Stejna pravidla jako ISINT.
 */
void codegen_isints(void);

// ---------- Section 7.4.8 from spec
/**
 * @brief Prints LABEL (defines a label at the currentposition in the code)
 *
 * @param label_literal name of the label "$while"
 *
 * @note Redefinice stejneho navesti (error 52)
 */
void codegen_label(const char *label_literal);

/**
 * @brief Prints JUMP (jump to a label)
 *
 * @param label Name of target label
 *
 * @note Skok na neexistujici navesti (error 52)
 */
void codegen_jump(const char *label);

/**
 * @brief Prints JUMPIFEQ (jumps to label if source1 = source2)
 *
 * @param label Name of the target lable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Skok na neexistujici navesti (error 52)
 * @note Operandy musi byt stejneho typu, nebo jeden z nich nil, jinak (error
 * 53)
 */
void codegen_jumpifeq(const char *label, const char *source1,
                      const char *source2);

/**
 * @brief Prints JUMPIFNEQ (jumps to label if source1 != source2)
 *
 * @param label Name of the target lable
 * @param source1 First operand
 * @param source2 Second operand
 *
 * @note Skok na neexistujici navesti (error 52)
 * @note Operandy musi byt stejneho typu, nebo jeden z nich nil, jinak (error
 * 53)
 */
void codegen_jumpifneq(const char *label, const char *source1,
                       const char *source2);

/**
 * @brief Prints EXIT (exits interpreter with return code)
 *
 * @param code Return code (int, variable or const)
 *
 * @note Hodnota mimo 0-49 (error 57)
 */
void codegen_exit(const char *code);

// ---------- Section 7.4.8 stack instructions
/**
 * @brief Prints JUMPIFEQS (stack version of JUMPIFEQ)
 *
 * @param label Name of the target label
 *
 * @note Vybere dve hodnoty ze zasobniku a skoci, pokud se rovnaji
 * @note Stejna pravidla a chyby jako JUMPIFEQ.
 */
void codegen_jumpifeqs(const char *label);

/**
 * @brief Prints JUMPIFNEQS (stack version of JUMPIFNEQ)
 *
 * @param label Name of the target label
 *
 * @note Vybere dve hodnoty ze zasobniku a skoci, pokud se nerovnaji
 * @note Stejna pravidla a chyby jako JUMPIFNEQ.
 */
void codegen_jumpifneqs(const char *label);

// ---------- Section 7.4.9 from spec

/**
 * @brief Prints BREAK (prints state to stderr)
 */
void codegen_break(void);

/**
 * @brief Prints DPRINT (prints value to stderr)
 *
 * @param symbol Value to print
 */
void codegen_dprint(const char *symbol);

// ---------- Unique label id generator
/**
 * @brief Returns new label id (int)
 *
 * @return Id unique within the whole program
 */
int codegen_new_label_id(void);

// ---------- String literal escaping
/**
 * @brief Prints a string in IFJcode26 escaped form
 *
 * @param string String value
 *
 */
void codegen_print_escaped_string(const char *string);

// ---------- Constant formating
/**
 * @brief Builds integer constant operand "int@integer"
 *
 * @param value Integer value
 *
 * @return Newly allocated string, or NULL on failure. Caller must free() it.
 */
char *codegen_const_int(long long value);

/**
 * @brief Builds bool constant operand "bool@true/false"
 *
 * @param value Boolean value
 *
 * @return Newly allocated string, or NULL on failure. Caller must free() it.
 */
char *codegen_const_bool(bool value);

/**
 * @brief Builds float constant operand "float@0x1.26666p+0"
 *
 * @param value Floating point value
 *
 * @return Newly allocated string, or NULL on failure. Caller must free() it.
 */
char *codegen_const_float(double value);

/**
 * @brief Builds the nil constant operand "nil@nil"
 *
 * @return Newly allocated string, or NULL on failure. Caller must free() it.
 */
char *codegen_const_nil(void);

/**
 * @brief Builds string constant operand "string@ahoj\032svete"
 *
 * @param string String value
 *
 * @return Newly allocated string, or NULL on failure. Caller must free() it.
 */
char *codegen_const_string(const char *string);
#endif
