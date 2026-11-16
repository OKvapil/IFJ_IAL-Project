/**
 * @file parser.c
 * @brief IFJ26 parser
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */

#include "parser.h"
#include "ast.h"
#include "errors.h"
#include "expressions.h"
#include "scanner.h"
#include "token.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Token current_token;

static AstNode *parse_program(void);

static AstNode *parse_func_call(char *name, size_t line);

static void parse_import(void);

static AstNode *parse_statement(void);

static AstNode *parse_block(void);

static DataType parse_type(void);

static AstNode *parse_func_def(bool is_main);

static AstNode *parse_return_state(void);

static AstNode *parse_if_state(void);

static AstNode *parse_while_do_state(void);

static AstNode *parse_for_state(void);

static AstNode *parse_params(void);

static AstNode *parse_func_main(void);

static AstNode *parse_value_or_if(void);

/**
 * @brief Function expects token, if current_token is correct (expected), we
 * clear current token with token_clear function and get next token from
 * scanner_get_next_token function and end a function.
 * If token is not expected, we print an error message and exit function with
 * error code ERR_SYNTAX.
 * If next token after expected_token dont return ERR_OK, we exit with
 * ERR_LEXICAL.
 * @param expected_token Expected token on input
 **/
static void check_token(TokenType expected_token) {
  if (current_token.type == expected_token) {
    token_clear(&current_token);
    int lex_res = scanner_get_next_token(&current_token);
    if (lex_res != ERR_OK) {
      exit(ERR_LEXICAL);
    }
  } else {
    fprintf(stderr,
            "Syntax error: expected token type %d but got %d at line %zu\n",
            expected_token, current_token.type, current_token.line);
    exit(ERR_SYNTAX);
  }
}

/**
 * @brief Function will return an identifier if current_token.type equals to
 * TOKEN_IDENTIFIER, if not, function will exit with ERR_SYNTAX.
 * @return identifier(name)
 **/
static char *consume_identifier(void) {
  if (current_token.type != TOKEN_IDENTIFIER) {
    fprintf(stderr, "Syntax error: expected identifier at line %zu\n",
            current_token.line);
    exit(ERR_SYNTAX);
  }
  char *name = current_token.attr.str_val;
  current_token.attr.str_val = NULL;
  check_token(TOKEN_IDENTIFIER);
  return name;
}

AstNode *parser_parse(void) {
  if (scanner_get_next_token(&current_token) != ERR_OK) {
    exit(ERR_LEXICAL);
  }
  return parse_program();
}

/**
 * @brief Function is inside of parentheses, reading the arguments until we end
 * up reading right parenthesis
 * @param name identifier of function
 * @param line line number
 * @return identifier(name)
 **/
static AstNode *parse_func_call(char *name, size_t line) {
  check_token(TOKEN_LPAREN);
  AstNode *args_block = ast_block();

  if (current_token.type != TOKEN_RPAREN) {
    while (true) {
      AstNode *arg = parse_expression(&current_token);
      ast_block_add_stmt(args_block, arg);
      if (current_token.type == TOKEN_COMMA) {
        check_token(TOKEN_COMMA);
      } else {
        break;
      }
    }
  }
  check_token(TOKEN_RPAREN);

  size_t arg_count = args_block->data.block.count;
  AstNode **args = args_block->data.block.statements;
  free(args_block);
  return ast_func_call(name, args, arg_count, line);
}

/**
 * @brief Function expect there is import keyword at the beginning of the file,
 * then go inside the program block and continue until it stops at EOF.
 * Function also checks if is inside the main func or def func, if not,
 * exit with ERR_SYNTAX
 * @return complete AstNode of program
 **/
static AstNode *parse_program(void) {
  parse_import();
  AstNode *program = ast_block();
  while (current_token.type != TOKEN_EOF) {
    if (current_token.type == TOKEN_EOL) {
      check_token(TOKEN_EOL);
      continue;
    }
    switch (current_token.type) {
    case TOKEN_ANNOT_MAIN:
      ast_block_add_stmt(program, parse_func_main());
      break;
    case TOKEN_KW_DEF:
      ast_block_add_stmt(program, parse_func_def(false));
      break;
    default:
      fprintf(stderr,
              "Syntax error: Expected function definition at line %zu\n",
              current_token.line);
      exit(ERR_SYNTAX);
    }
  }
  return program;
}

/**
 * @brief Function checks the correct order of "import" in the beggining of
 * file.
 **/
static void parse_import(void) {
  check_token(TOKEN_KW_IMPORT);
  check_token(TOKEN_IDENTIFIER);
  check_token(TOKEN_DOT);
  check_token(TOKEN_MUL);
}

/**
 * @brief Parses either an if-expression or a standard expression.
 * @return Pointer to the parsed AstNode.
 **/
static AstNode *parse_value_or_if(void) {
  if (current_token.type == TOKEN_KW_IF) {
    return parse_if_state();
  }
  return parse_expression(&current_token);
}

/**
 * @brief Function checks different types of keyword tokens, for every type
 * decide how this keyword behaves and returns function for its next processing
 * @return valid AstNode statement
 **/
static AstNode *parse_statement(void) {
  size_t line = current_token.line;
  switch (current_token.type) {
  case TOKEN_KW_VAL:
  case TOKEN_KW_VAR: {
    AstKind kind = (current_token.type == TOKEN_KW_VAL) ? AST_VAL : AST_VAR;
    check_token(current_token.type);
    char *name = consume_identifier();
    DataType type = TYPE_UNKNOWN;
    if (current_token.type == TOKEN_COLON) {
      check_token(TOKEN_COLON);
      type = parse_type();
    }
    check_token(TOKEN_ASSIGN);
    AstNode *init_expr = parse_value_or_if();
    return ast_var_decl(kind, name, type, init_expr, line);
  }
  case TOKEN_KW_IF:
    return parse_if_state();
  case TOKEN_KW_WHILE:
    return parse_while_do_state();
  case TOKEN_KW_FOR:
    return parse_for_state();
  case TOKEN_KW_RETURN:
    return parse_return_state();
  case TOKEN_IDENTIFIER: {
    char *name = consume_identifier();
    if (current_token.type == TOKEN_ASSIGN) {
      check_token(TOKEN_ASSIGN);
      AstNode *value = parse_value_or_if();
      return ast_expr_assign(name, value, line);
    }
    AstNode *first;
    if (current_token.type == TOKEN_LPAREN) {
      first = parse_func_call(name, line);
    } else {
      Token id;
      id.type = TOKEN_IDENTIFIER;
      id.line = line;
      id.attr.str_val = name;
      first = ast_from_token(&id);
    }
    return parse_expression_from(&current_token, first);
  }
  default:
    fprintf(stderr, "Syntax error: expected statement at line %zu\n",
            current_token.line);
    exit(ERR_SYNTAX);
  }
}

/**
 * @brief Function checks different types of keyword tokens, for every type
 * decide how this keyword behaves and returns function for its next processing
 * @return block
 **/
static AstNode *parse_block(void) {
  check_token(TOKEN_EOL);
  check_token(TOKEN_INDENT);
  AstNode *block = ast_block();
  while (current_token.type != TOKEN_DEDENT) {
    if (current_token.type == TOKEN_EOL) {
      check_token(TOKEN_EOL);
    } else {
      AstNode *stmt = parse_statement();
      ast_block_add_stmt(block, stmt);
      if (current_token.type == TOKEN_SEMICOLON) {
        check_token(TOKEN_SEMICOLON);
      }
    }
  }
  check_token(TOKEN_DEDENT);
  return block;
}

/**
 * @brief check the current token and return it, if token is invalid, exit with
 * ERR_SYNTAX
 * @return data type
 **/
static DataType parse_type(void) {
  switch (current_token.type) {
  case TOKEN_KW_INT:
    check_token(TOKEN_KW_INT);
    return TYPE_INT;
  case TOKEN_KW_STRING:
    check_token(TOKEN_KW_STRING);
    return TYPE_STRING;
  case TOKEN_KW_DOUBLE:
    check_token(TOKEN_KW_DOUBLE);
    return TYPE_DOUBLE;
  case TOKEN_KW_UNIT:
    check_token(TOKEN_KW_UNIT);
    return TYPE_UNIT;
  case TOKEN_KW_BOOLEAN:
    check_token(TOKEN_KW_BOOLEAN);
    return TYPE_BOOL;
  default:
    fprintf(stderr, "Syntax error: expected type at line %zu\n",
            current_token.line);
    exit(ERR_SYNTAX);
  }
}

/**
 * @brief function for func definition, it checks if definition has correct
 * syntax, if yes, it pass arguments in ast_func_def
 * @param is_main checks if func is main
 * @return AstNode of func definition
 **/
static AstNode *parse_func_def(bool is_main) {
  size_t line = current_token.line;
  check_token(TOKEN_KW_DEF);
  char *name = consume_identifier();
  check_token(TOKEN_LPAREN);
  AstNode *params = parse_params();
  check_token(TOKEN_RPAREN);
  DataType return_type = TYPE_UNKNOWN;
  if (current_token.type == TOKEN_COLON) {
    check_token(TOKEN_COLON);
    return_type = parse_type();
  }
  check_token(TOKEN_ASSIGN);

  AstNode *block = NULL;
  if (current_token.type == TOKEN_EOL) {
    block = parse_block();
  } else {
    block = parse_value_or_if();
  }

  AstNode **param_list = params->data.block.statements;
  size_t param_count = params->data.block.count;
  free(params);

  return ast_func_def(name, return_type, param_list, param_count, block,
                      is_main, line);
}

/**
 * @brief function returns next token through parse_expression function,
 * if return contains token from condition, it returns NULL
 * @return AstNode to ast_return func
 **/
static AstNode *parse_return_state(void) {
  size_t line = current_token.line;
  check_token(TOKEN_KW_RETURN);
  if (current_token.type != TOKEN_EOL && current_token.type != TOKEN_DEDENT &&
      current_token.type != TOKEN_EOF &&
      current_token.type != TOKEN_SEMICOLON) {
    AstNode *expr = parse_value_or_if();
    return ast_return(expr, line);
  }
  return ast_return(NULL, line);
}

/**
 * @brief function check "if" keyword, condition after it, then keyword "then"
 *and block of statements. If it has "else" block, function check the block too.
 * @return AstNode to ast_if func
 **/
static AstNode *parse_if_state(void) {
  size_t line = current_token.line;
  check_token(TOKEN_KW_IF);
  AstNode *cond = parse_expression(&current_token);
  check_token(TOKEN_KW_THEN);
  AstNode *then_block = NULL;
  if (current_token.type == TOKEN_EOL) {
    then_block = parse_block();
  } else {
    then_block = parse_value_or_if();
  }
  while (current_token.type == TOKEN_EOL) {
    check_token(TOKEN_EOL);
  }
  AstNode *else_block = NULL;
  if (current_token.type == TOKEN_KW_ELSE) {
    check_token(TOKEN_KW_ELSE);
    if (current_token.type == TOKEN_EOL) {
      else_block = parse_block();
    } else {
      else_block = parse_value_or_if();
    }
  }
  return ast_if(cond, then_block, else_block, line);
}

/**
 * @brief function check "while" keyword, condition after it,
 * "do" keyword and block
 * @return AstNode to ast_while func
 **/
static AstNode *parse_while_do_state(void) {
  size_t line = current_token.line;
  check_token(TOKEN_KW_WHILE);
  AstNode *cond = parse_expression(&current_token);
  check_token(TOKEN_KW_DO);
  AstNode *block = parse_block();
  return ast_while(cond, block, line);
}

static AstNode *parse_for_state(void) {
  size_t line = current_token.line;
  check_token(TOKEN_KW_FOR);

  bool multiline = false;
  if (current_token.type == TOKEN_EOL) {
    multiline = true;
    while (current_token.type == TOKEN_EOL) {
      check_token(TOKEN_EOL);
    }
    check_token(TOKEN_INDENT);
  }

  char *var_name = consume_identifier();
  check_token(TOKEN_ARROW);
  AstNode *from = parse_expression(&current_token);
  bool until = false;
  if (current_token.type == TOKEN_KW_TO) {
    check_token(TOKEN_KW_TO);
  } else if (current_token.type == TOKEN_KW_UNTIL) {
    check_token(TOKEN_KW_UNTIL);
    until = true;
  } else {
    fprintf(stderr, "Syntax error: expected 'to' or 'until' at line %zu\n",
            current_token.line);
    exit(ERR_SYNTAX);
  }
  AstNode *to = parse_expression(&current_token);

  AstNode *step = NULL;
  if (current_token.type == TOKEN_KW_BY) {
    check_token(TOKEN_KW_BY);
    step = parse_expression(&current_token);
  }
  if (multiline) {
    while (current_token.type == TOKEN_EOL) {
      check_token(TOKEN_EOL);
    }
  }
  AstNode *guards_block = ast_block();
  while (current_token.type == TOKEN_KW_IF) {
    check_token(TOKEN_KW_IF);
    AstNode *guard = parse_expression(&current_token);
    ast_block_add_stmt(guards_block, guard);
    if (multiline) {
      while (current_token.type == TOKEN_EOL) {
        check_token(TOKEN_EOL);
      }
    }
  }
  if (multiline) {
    check_token(TOKEN_DEDENT);
    while (current_token.type == TOKEN_EOL) {
      check_token(TOKEN_EOL);
    }
  }
  check_token(TOKEN_KW_DO);

  AstNode *body = NULL;
  if (current_token.type == TOKEN_EOL) {
    body = parse_block();
  } else {
    body = parse_statement();
  }

  AstNode **guards = guards_block->data.block.statements;
  size_t guard_count = guards_block->data.block.count;
  free(guards_block);

  return ast_for(line, var_name, from, to, step, until, guards, guard_count,
                 body);
}

/**
 * @brief function checks all parametres inside the parantheses of function
 * and expect them to be seperated by comma
 * @return AstNode to params_block function
 **/
static AstNode *parse_params(void) {
  AstNode *params_block = ast_block();
  if (current_token.type == TOKEN_RPAREN) {
    return params_block;
  }
  while (true) {
    size_t line = current_token.line;
    char *param_name = consume_identifier();
    check_token(TOKEN_COLON);
    DataType param_type = parse_type();
    AstNode *param = ast_var_decl(AST_VAL, param_name, param_type, NULL, line);
    ast_block_add_stmt(params_block, param);

    if (current_token.type == TOKEN_COMMA) {
      check_token(TOKEN_COMMA);
    } else {
      break;
    }
  }
  return params_block;
}
/**
 * @brief function expect the kw main ,after that jumps into the parse_func_def
 * func with true flag,
 * so the func knows that it should behave to this block as main
 * @return AstNode to parse_func_def func
 **/
static AstNode *parse_func_main(void) {
  check_token(TOKEN_ANNOT_MAIN);
  while (current_token.type == TOKEN_EOL) {
    check_token(TOKEN_EOL);
  }
  return parse_func_def(true);
}
