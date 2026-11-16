/**
* @file main.c
 * @brief Entry point of the IFJ26 compiler
 *
 * Reads IFJ26 source code from stdin, runs lexical, syntax and semantic
 * analysis and writes the generated IFJcode26 program to stdout.
 * Returns 0 on success or the corresponding error code.
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 * @author Filip Haza (xhazafi00)
 */

#include "ast.h"
#include "errors.h"
#include "generator.h"
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <semantic.h>

int main(void) {
  return 0;
}
