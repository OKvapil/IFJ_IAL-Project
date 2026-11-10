/**
 * @file expressions.c
 * @brief IFJcode26 parser expressions functions (implementation)
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
  LPAREN,
  RPAREN,
  ID,
  COMMA,
  DOLLAR,
  TAB_MAX
} OperatorsTable;

static OperatorsTable token_to_index(TokenType type) {
  switch (type) {
  case TOKEN_PLUS:
    return PLUS;
    break;
  case TOKEN_MINUS:
    return MINUS;
    break;
  case TOKEN_MUL:
    return MUL;
    break;
  case TOKEN_DIV:
    return DIV;
    break;
  case TOKEN_LPAREN:
    return LPAREN;
    break;
  case TOKEN_RPAREN:
    return RPAREN;
    break;
  case TOKEN_INT_LIT:
    return ID;
  case TOKEN_DOUBLE_LIT:
    return ID;
  case TOKEN_STRING_LIT:
    return ID;
  case TOKEN_IDENTIFIER:
    return ID;
  case TOKEN_COMMA:
    return COMMA;
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
    break;
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
    //          +  -  *  /  r  e  (  )  i  ,  $
    /* + */ {R, R, S, S, R, R, S, R, S, R, R},
    /* - */ {R, R, S, S, R, R, S, R, S, R, R},
    /* * */ {R, R, R, R, R, R, S, R, S, R, R},
    /* / */ {R, R, R, R, R, R, S, R, S, R, R},
    /* r */ {S, S, S, S, X, X, S, R, S, R, R},
    /* e */ {S, S, S, S, X, X, S, R, S, R, R},
    /* ( */ {S, S, S, S, S, S, S, E, S, S, X},
    /* ) */ {R, R, R, R, R, R, X, R, X, R, R},
    /* i */ {R, R, R, R, R, R, X, R, X, R, R},
    /* , */ {S, S, S, S, S, S, S, R, S, R, X},
    /* $ */ {S, S, S, S, S, S, S, X, S, X, X}};
// e = equality operators
// r = relation operators

static int reduce(Stack *s);

static void init_stack(Stack *s);

static int push_stack(Stack *s, SymbolType Type, OperatorsTable operator,
                      TokenType token_type, size_t line, AstNode *ast_node);

static void pop_stack(Stack *s);

static StackNode *stack_top_terminal(Stack *s);

static void stack_push_after_top_terminal(Stack *s);

static void stack_free(Stack *s);

static int move_next_token(Token *token);

AstNode *parse_expression(Token *token);

static int reduce(Stack *s) {
  if (s->top == NULL)
    return ERR_SYNTAX;

  // Pravidlo E->i
  if (s->top->type == SYMBOL_TERMINAL && s->top->operator== ID &&
                                         s->top->next != NULL &&
                                         s->top->next->type == SYMBOL_STOP) {
    AstNode *node = s->top->ast_node;
    pop_stack(s);
    pop_stack(s);
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
    pop_stack(s);
    pop_stack(s);
    pop_stack(s);
    pop_stack(s);
    return push_stack(s, SYMBOL_NONTERMINAL, DOLLAR, TOKEN_EOF, 0, node);
  }
  // Pravidlo E-> E op E
  if (first->type == SYMBOL_NONTERMINAL && second->type == SYMBOL_TERMINAL &&
      third->type == SYMBOL_NONTERMINAL) {
    AstNode *node = ast_binary(second->token_type, third->ast_node,
                               first->ast_node, second->line);
    if (node == NULL)
      return ERR_INTERNAL;
    pop_stack(s);
    pop_stack(s);
    pop_stack(s);
    pop_stack(s);
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

static void pop_stack(Stack *s) {
  if (s->top == NULL)
    return;
  StackNode *temp = s->top;
  s->top = s->top->next;
  free(temp);
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
  if (terminal == s->top) {
    stop_node->token_type = TOKEN_EOF;
    stop_node->line = 0;
    stop_node->ast_node = NULL;
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
  token_clear(token);
  return scanner_get_next_token(token);
}

AstNode *parse_expression(Token *token) {
  Stack stack;
  init_stack(&stack);
  push_stack(&stack, SYMBOL_TERMINAL, DOLLAR, TOKEN_EOF, 0, NULL);
  while (true) {
    StackNode *upper = stack_top_terminal(&stack);
    OperatorsTable a = upper->operator;
    OperatorsTable b = token_to_index(token->type);
    if (a == DOLLAR && b == DOLLAR)
      break;
    switch (precedence_table[a][b]) {
    case S:
      stack_push_after_top_terminal(&stack);
      if (b == ID) {
        AstNode *leaf = ast_from_token(token);
        push_stack(&stack, SYMBOL_TERMINAL, b, token->type, token->line, leaf);
      } else {
        push_stack(&stack, SYMBOL_TERMINAL, b, token->type, token->line, NULL);
      }
      if (move_next_token(token) != ERR_OK) {
        stack_free(&stack);
        exit(ERR_LEXICAL);
      }
      break;
    case R:
      if (reduce(&stack) != ERR_OK) {
        stack_free(&stack);
        exit(ERR_SYNTAX);
      }
      break;
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
      break;
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
