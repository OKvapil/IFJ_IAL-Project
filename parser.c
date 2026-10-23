/**
 * @file parser.c
 * @brief IFJcode26 parser (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */

#include "parser.h"
#include "scanner.h"
#include "token.h"

static Token current_token;

static void check_token(TokenType expected_token);
static void parse_import(void);
static void parse_program(void);
static void parse_statement(void);
static void parse_expression(void);
static void parse_block(void);
static void parse_type(void);
//
static void parse_func_def(void);
static void parse_return_state(void);
static void parse_if_state(void);
static void parse_while_do_state(void);
static void parse_params(void);
static void parse_func_main(void);

static void check_token(TokenType expected_token) {
  if (current_token.type == expected_token) {
    token_clear(&current_token);
    int lex_res = scanner_get_next_token(&current_token);
    if (lex_res != 0) {
      exit(1);
    }
  } else {
    fprintf(stderr,
            "Syntax error: expected token type %d but got %d at line %zu\n",
            expected_token, current_token.type, current_token.line);
    exit(2);
  }
}

static void parse_program(void) {
  parse_import();
  while (current_token.type != TOKEN_EOF) {
    if (current_token.type == TOKEN_EOL) {
      check_token(TOKEN_EOL);
      continue;
    }
    switch (current_token.type) {
    case TOKEN_ANNOT_MAIN:
      parse_func_main();
      break;
    case TOKEN_KW_DEF:
      parse_func_def();
      break;
    default:
      fprintf(stderr,
              "Syntax error: Expected function definition at line %zu\n",
              current_token.line);
      exit(2);
    }
  }
}

static void parse_import(void) {
  check_token(TOKEN_KW_IMPORT);
  check_token(TOKEN_IDENTIFIER);
  check_token(TOKEN_DOT);
  check_token(TOKEN_MUL);
}

static void parse_statement(void) {
  switch (current_token.type) {
  case TOKEN_KW_VAL:
    check_token(TOKEN_KW_VAL);
    break;
  case TOKEN_KW_VAR:
    check_token(TOKEN_KW_VAR);
    break;
  case TOKEN_KW_IF:
    parse_if_state();
    break;
  case TOKEN_KW_WHILE:
    parse_while_do_state();
    break;
  default:
  }
}

static void parse_expression(void) {
  // Implement expression parsing logic here
}

static void parse_block(void) {
  // Implement block parsing logic here
}

static void parse_type(void) {
  switch (current_token.type) {
  case TOKEN_KW_INT:
    check_token(TOKEN_KW_INT);
    break;
  case TOKEN_KW_STRING:
    check_token(TOKEN_KW_STRING);
    break;
  case TOKEN_KW_UNIT:
    check_token(TOKEN_KW_UNIT);
    break;
  case TOKEN_KW_DOUBLE:
    check_token(TOKEN_KW_DOUBLE);
    break;
  case TOKEN_KW_VAL:
    check_token(TOKEN_KW_VAL);
    break;
  case TOKEN_KW_VAR:
    check_token(TOKEN_KW_VAR);
    break;
  default:
  }
}

static void parse_func_def(void) {
  check_token(TOKEN_KW_DEF);
  check_token(TOKEN_IDENTIFIER);
  check_token(TOKEN_LPAREN);
  parse_params();
  check_token(TOKEN_RPAREN);
  check_token(TOKEN_COLON);
  parse_type();
  check_token(TOKEN_ASSIGN);
  switch (current_token.type) {
  case TOKEN_EOL:
    parse_block();
    break;
  default:
    parse_expression();
  }
}

static void parse_return_state(void) {
  check_token(TOKEN_KW_RETURN);
  if (current_token.type != TOKEN_EOL && current_token.type != TOKEN_DEDENT) {
    parse_expression();
  }
}
static void parse_if_state(void) {
  check_token(TOKEN_KW_IF);
  parse_expression();
  check_token(TOKEN_KW_THEN);
  parse_block();
  check_token(TOKEN_KW_ELSE);
  parse_block();
}
static void parse_while_do_state(void) {
  check_token(TOKEN_KW_WHILE);
  parse_expression();
  check_token(TOKEN_KW_DO);
  parse_block();
}

static void parse_params(void) {
  check_token(TOKEN_IDENTIFIER);
  check_token(TOKEN_COLON);
  parse_type();
  check_token(TOKEN_RPAREN);
}

static void parse_func_main() {
  check_token(TOKEN_ANNOT_MAIN);
  while (current_token.type == TOKEN_EOL) {
    check_token(TOKEN_EOL);
  }
  parse_func_def();
}