/**
 * @file expressions.c
 * @brief IFJ26 parser expressions functions
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */

#include "expressions.h"
#include "ast.h"
#include "errors.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum { SYMBOL_TERMINAL, SYMBOL_NONTERMINAL, SYMBOL_STOP } SymbolType;

typedef enum {
  S, // Shift <
  R, // Reduce >
  E, // Equal =
  X  // Syntax error
} Operations;

typedef enum {
  PLUS,
  MINUS,
  MUL,
  DIV,
  REL,
  EQ,
  AND,
  OR,
  NOT,
  UMINUS,
  LPAREN,
  RPAREN,
  ID,
  DOLLAR,
  TAB_MAX
} OperatorsTable;

static OperatorsTable token_to_index(TokenType type) {
  switch (type) {
  case TOKEN_PLUS:
    return PLUS;
  case TOKEN_MINUS:
    return MINUS;
  case TOKEN_MUL:
    return MUL;
  case TOKEN_DIV:
    return DIV;
  case TOKEN_AND:
    return AND;
  case TOKEN_OR:
    return OR;
  case TOKEN_NOT:
    return NOT;
  case TOKEN_LPAREN:
    return LPAREN;
  case TOKEN_RPAREN:
    return RPAREN;
  case TOKEN_INT_LIT:
    return ID;
  case TOKEN_DOUBLE_LIT:
    return ID;
  case TOKEN_STRING_LIT:
    return ID;
  case TOKEN_IDENTIFIER:
    return ID;
  case TOKEN_KW_TRUE:
    return ID;
  case TOKEN_KW_FALSE:
    return ID;
  case TOKEN_KW_IF:
    return ID;
  case TOKEN_LT:
    return REL;
  case TOKEN_LTE:
    return REL;
  case TOKEN_GT:
    return REL;
  case TOKEN_GTE:
    return REL;
  case TOKEN_EQ:
    return EQ;
  case TOKEN_NEQ:
    return EQ;
  default:
    return DOLLAR;
  }
}

typedef struct StackNode {
  SymbolType type;
  OperatorsTable operator;
  TokenType token_type;
  size_t line;
  AstNode *ast_node;
  struct StackNode *next;
} StackNode;

typedef struct {
  StackNode *top;
} Stack;

static const Operations precedence_table[TAB_MAX][TAB_MAX] = {
    //       +  -  *  /  r  e  &  |  !  u  (  )  i  $
    /* + */ {R, R, S, S, R, R, R, R, S, S, S, R, S, R},
    /* - */ {R, R, S, S, R, R, R, R, S, S, S, R, S, R},
    /* * */ {R, R, R, R, R, R, R, R, S, S, S, R, S, R},
    /* / */ {R, R, R, R, R, R, R, R, S, S, S, R, S, R},
    /* r */ {S, S, S, S, X, R, R, R, S, S, S, R, S, R},
    /* e */ {S, S, S, S, S, X, R, R, S, S, S, R, S, R},
    /* & */ {S, S, S, S, S, S, R, R, S, S, S, R, S, R},
    /* | */ {S, S, S, S, S, S, S, R, S, S, S, R, S, R},
    /* ! */ {R, R, R, R, R, R, R, R, S, S, S, R, S, R},
    /* u */ {R, R, R, R, R, R, R, R, S, S, S, R, S, R},
    /* ( */ {S, S, S, S, S, S, S, S, S, S, S, E, S, X},
    /* ) */ {R, R, R, R, R, R, R, R, X, X, X, R, X, R},
    /* i */ {R, R, R, R, R, R, R, R, X, X, X, R, X, R},
    /* $ */ {S, S, S, S, S, S, S, S, S, S, S, X, S, X}};

// e = equality operators
// r = relation operators

static int reduce(Stack *s);

static void init_stack(Stack *s);

static int push_stack(Stack *s, SymbolType Type, OperatorsTable operator,
                      TokenType token_type, size_t line, AstNode *ast_node);

static void pop_stack(Stack *s, unsigned n);

static StackNode *stack_top_terminal(Stack *s);

static void stack_push_after_top_terminal(Stack *s);

static void stack_free(Stack *s);

static int move_next_token(Token *token);

static AstNode *parse_expression_impl(Token *token, AstNode *first_operand);

static void err_state(Token *id_tok, AstNode *args_block, Stack *stack,
                      int exit_code);

static int reduce(Stack *s) {
  if (s->top == NULL)
    return ERR_SYNTAX;

  // Pravidlo E->i
  if (s->top->type == SYMBOL_TERMINAL && s->top->operator== ID &&
                                         s->top->next != NULL &&
                                         s->top->next->type == SYMBOL_STOP) {
    AstNode *node = s->top->ast_node;
    pop_stack(s, 2);
    return push_stack(s, SYMBOL_NONTERMINAL, DOLLAR, TOKEN_EOF, 0, node);
  }

  // pravidlo E -> -E / +E / !E
  if (s->top->type == SYMBOL_NONTERMINAL && s->top->next != NULL &&
      s->top->next->type == SYMBOL_TERMINAL &&
      (s->top->next->operator== UMINUS || s->top->next->operator== NOT) &&
      s->top->next->next != NULL && s->top->next->next->type == SYMBOL_STOP) {
    AstNode *operand = s->top->ast_node;
    TokenType op_tok = s->top->next->token_type;
    size_t op_line = s->top->next->line;
    AstNode *node = ast_unary(op_tok, operand, op_line);
    if (node == NULL)
      return ERR_INTERNAL;
    pop_stack(s, 3);
    return push_stack(s, SYMBOL_NONTERMINAL, DOLLAR, TOKEN_EOF, 0, node);
  }

  if (s->top->type == SYMBOL_TERMINAL &&
      s->top->operator== RPAREN && s->top->next != NULL &&
      s->top->next->type == SYMBOL_TERMINAL &&
      s->top->next->operator== LPAREN && s->top->next->next != NULL &&
      s->top->next->next->type == SYMBOL_STOP) {
    AstNode *node = ast_unit(s->top->next->line);
    if (node == NULL) {
      return ERR_INTERNAL;
    }
    pop_stack(s, 3);
    return push_stack(s, SYMBOL_NONTERMINAL, DOLLAR, TOKEN_EOF, 0, node);
  }
  StackNode *first = s->top;
  StackNode *second = (first != NULL) ? first->next : NULL;
  StackNode *third = (second != NULL) ? second->next : NULL;

  if (third == NULL || third->next == NULL ||
      third->next->type != SYMBOL_STOP) {
    return ERR_SYNTAX;
  }

  // Pravidlo E -> (E)
  if (first->type == SYMBOL_TERMINAL &&
      first->operator== RPAREN && second->type == SYMBOL_NONTERMINAL &&
      third->type == SYMBOL_TERMINAL && third->operator== LPAREN) {
    AstNode *node = second->ast_node;
    pop_stack(s, 4);
    return push_stack(s, SYMBOL_NONTERMINAL, DOLLAR, TOKEN_EOF, 0, node);
  }
  // Pravidlo E-> E op E
  if (first->type == SYMBOL_NONTERMINAL && second->type == SYMBOL_TERMINAL &&
      (second->operator== PLUS || second->operator== MINUS ||
       second->operator== MUL || second->operator== DIV ||
       second->operator== REL || second->operator== EQ ||
       second->operator== AND || second->operator== OR) &&
      third->type == SYMBOL_NONTERMINAL) {
    AstNode *node = ast_binary(second->token_type, third->ast_node,
                               first->ast_node, second->line);
    if (node == NULL)
      return ERR_INTERNAL;
    pop_stack(s, 4);
    return push_stack(s, SYMBOL_NONTERMINAL, DOLLAR, TOKEN_EOF, 0, node);
  }

  return ERR_SYNTAX;
}

static void init_stack(Stack *s) { s->top = NULL; }

static int push_stack(Stack *s, SymbolType Type, OperatorsTable operator,
                      TokenType token_type, size_t line, AstNode *ast_node) {
  StackNode *new_node = malloc(sizeof(StackNode));
  if (new_node == NULL) {
    exit(ERR_INTERNAL);
  }

  new_node->type = Type;
  new_node->operator= operator;
  new_node->token_type = token_type;
  new_node->line = line;
  new_node->ast_node = ast_node;
  new_node->next = s->top;
  s->top = new_node;
  return ERR_OK;
}

static void pop_stack(Stack *s, unsigned n) {
  for (unsigned i = 0; i < n; i++) {
    if (s->top == NULL)
      return;
    StackNode *temp = s->top;
    s->top = s->top->next;
    free(temp);
  }
}

static StackNode *stack_top_terminal(Stack *s) {
  StackNode *pos_pointer = s->top;
  while (pos_pointer != NULL) {
    if (pos_pointer->type == SYMBOL_TERMINAL) {
      return pos_pointer;
    }
    pos_pointer = pos_pointer->next;
  }
  return NULL;
}

static void stack_push_after_top_terminal(Stack *s) {
  StackNode *terminal = stack_top_terminal(s);
  if (terminal == NULL) {
    return;
  }
  StackNode *stop_node = malloc(sizeof(StackNode));
  if (stop_node == NULL) {
    exit(ERR_INTERNAL);
  }
  stop_node->type = SYMBOL_STOP;
  stop_node->operator= DOLLAR;
  stop_node->token_type = TOKEN_EOF;
  stop_node->line = 0;
  stop_node->ast_node = NULL;

  if (terminal == s->top) {
    stop_node->next = s->top;
    s->top = stop_node;
  } else {
    StackNode *above_terminal = s->top;
    while (above_terminal->next != terminal) {
      above_terminal = above_terminal->next;
    }
    stop_node->next = terminal;
    above_terminal->next = stop_node;
  }
}

static void stack_free(Stack *s) {
  if (s == NULL)
    return;

  StackNode *pointer = s->top;

  while (pointer != NULL) {
    StackNode *temp = pointer;
    if (temp->ast_node != NULL) {
      ast_free(temp->ast_node);
    }
    pointer = pointer->next;
    free(temp);
  }
  s->top = NULL;
}

static int move_next_token(Token *token) {
  if (token->type != TOKEN_INDENT && token->type != TOKEN_DEDENT &&
      token->type != TOKEN_EOL) {
    last_token_line = token->line;
  }
  token_clear(token);
  return scanner_get_next_token(token);
}

static AstNode *parse_expression_impl(Token *token, AstNode *first_operand) {
  Stack stack;
  init_stack(&stack);
  push_stack(&stack, SYMBOL_TERMINAL, DOLLAR, TOKEN_EOF, 0, NULL);
  if (first_operand != NULL) {
    push_stack(&stack, SYMBOL_NONTERMINAL, DOLLAR, TOKEN_EOF, 0, first_operand);
  }
  while (true) {
    StackNode *upper = stack_top_terminal(&stack);
    OperatorsTable a = upper->operator;
    OperatorsTable b = token_to_index(token->type);
    if (token->type == TOKEN_KW_IF &&
        (stack.top->type == SYMBOL_NONTERMINAL ||
         stack.top->operator== RPAREN || stack.top->operator== ID)) {
      b = DOLLAR;
    }
    if ((b == MINUS || b == PLUS) && stack.top->type == SYMBOL_TERMINAL &&
        stack.top->operator!= RPAREN && stack.top->operator!= ID) {
      b = UMINUS;
    }

    if (a == DOLLAR && (b == DOLLAR || b == RPAREN))
      break;

    switch (precedence_table[a][b]) {
    case S: {
      stack_push_after_top_terminal(&stack);
      if (b == ID) {
        if (token->type == TOKEN_KW_IF) {
          AstNode *leaf = parse_if_state();
          if (leaf == NULL) {
            err_state(NULL, NULL, &stack, ERR_INTERNAL);
          }
          if (token != &current_token) {
            *token = current_token;
          }
          push_stack(&stack, SYMBOL_TERMINAL, b, TOKEN_KW_IF, leaf->line, leaf);
          break;
        }

        AstNode *leaf = NULL;
        Token id_tok = *token;
        token->attr.str_val = NULL;

        if (move_next_token(token) != ERR_OK) {
          err_state(&id_tok, NULL, &stack, ERR_LEXICAL);
        }

        if (id_tok.type == TOKEN_IDENTIFIER && token->type == TOKEN_LPAREN) {
          if (move_next_token(token) != ERR_OK) {
            err_state(&id_tok, NULL, &stack, ERR_LEXICAL);
          }

          AstNode *args_block = ast_block();
          if (token->type != TOKEN_RPAREN) {
            while (true) {
              AstNode *arg = parse_expression(token);
              ast_block_add_stmt(args_block, arg);
              if (token->type != TOKEN_COMMA)
                break;
              size_t comma_line = token->line;
              if (move_next_token(token) != ERR_OK)
                err_state(&id_tok, args_block, &stack, ERR_LEXICAL);
              if (token->type == TOKEN_RPAREN) {
                if (token->line > comma_line) {
                  break;
                } else {
                  fprintf(stderr,
                          "Syntax error: trailing comma requires newline "
                          "before ')' at line %zu\n",
                          token->line);
                  err_state(&id_tok, args_block, &stack, ERR_SYNTAX);
                }
              }
            }
          }
          if (token->type != TOKEN_RPAREN) {
            err_state(&id_tok, args_block, &stack, ERR_SYNTAX);
          }
          if (move_next_token(token) != ERR_OK) {
            err_state(&id_tok, args_block, &stack, ERR_LEXICAL);
          }

          leaf = ast_func_call(id_tok.attr.str_val,
                               args_block->data.block.statements,
                               args_block->data.block.count, id_tok.line);
          if (leaf == NULL) {
            err_state(&id_tok, args_block, &stack, ERR_INTERNAL);
          }
          free(args_block);
        } else {
          leaf = ast_from_token(&id_tok);
          if (leaf == NULL) {
            err_state(&id_tok, NULL, &stack, ERR_INTERNAL);
          }
        }
        push_stack(&stack, SYMBOL_TERMINAL, b, id_tok.type, id_tok.line, leaf);
      } else {
        push_stack(&stack, SYMBOL_TERMINAL, b, token->type, token->line, NULL);
        if (move_next_token(token) != ERR_OK) {
          stack_free(&stack);
          exit(ERR_LEXICAL);
        }
      }
      break;
    }
    case R: {
      int rc = reduce(&stack);
      if (rc != ERR_OK) {
        stack_free(&stack);
        exit(rc);
      }
      break;
    }
    case E:
      push_stack(&stack, SYMBOL_TERMINAL, b, token->type, token->line, NULL);
      if (move_next_token(token) != ERR_OK) {
        stack_free(&stack);
        exit(ERR_LEXICAL);
      }
      break;
    case X:
      stack_free(&stack);
      exit(ERR_SYNTAX);
    }
  }
  if (stack.top != NULL && stack.top->type == SYMBOL_NONTERMINAL &&
      stack.top->next != NULL && stack.top->next->type == SYMBOL_TERMINAL &&
      stack.top->next->operator== DOLLAR && stack.top->next->next == NULL) {
    AstNode *result = stack.top->ast_node;
    stack.top->ast_node = NULL;
    stack_free(&stack);
    return result;
  } else {
    stack_free(&stack);
    exit(ERR_SYNTAX);
  }
}

AstNode *parse_expression(Token *token) {
  return parse_expression_impl(token, NULL);
}

AstNode *parse_expression_from(Token *token, AstNode *first_operand) {
  return parse_expression_impl(token, first_operand);
}

static void err_state(Token *id_tok, AstNode *args_block, Stack *stack,
                      int exit_code) {
  token_clear(id_tok);
  ast_free(args_block);
  stack_free(stack);
  exit(exit_code);
}