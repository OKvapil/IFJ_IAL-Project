/**
 * @file parser.c
 * @brief IFJcode26 parser expressions functions (implementation)
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Petr Nášel (xnaselp00)
 */


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
  case TOKEN_FLOAT_LIT:
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
  struct StackNode *next;
} StackNode;

typedef struct {
  StackNode *top;
} Stack;

static void init_stack(Stack *s) { s->top = NULL; }

static void push_stack(Stack *s, SymbolType Type, OperatorsTable operator) {
  StackNode *new_node = malloc(sizeof(StackNode));
  if (new_node == NULL) {
    return 99;
  }

  new_node->type = Type;
  new_node->next = s->top;
  new_node->operator= operator;
  s->top = new_node;
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
    return;
  }
  stop_node->type = SYMBOL_STOP;
  stop_node->operator= DOLLAR;
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
    pointer = pointer->next;
    free(temp);
  }
  s->top = NULL;
}

static const Operations precedence_table[TAB_MAX][TAB_MAX] = {
    //       +  -  *  /  r  e  (  )  i  ,  $
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

static int move_next_token(Token *token) {
  token_clear(token);
  return scanner_get_next_token(token);
}

int parse_expression(Token *token) {
  Stack stack;
  init_stack(&stack);
  push_stack(&stack, SYMBOL_TERMINAL, DOLLAR);
  while (true) {
    StackNode *upper = stack_top_terminal(&stack);
    OperatorsTable a = upper->operator;
    OperatorsTable b = token_to_index(token->type);
    if (a == DOLLAR && b == DOLLAR)

      break;
    switch (precedence_table[a][b]) {
    case S:
      stack_push_after_top_terminal(&stack);
      push_stack(&stack, SYMBOL_TERMINAL, b);
      if (move_next_token(token) == 1) {
        stack_free(&stack);
        return 1;
      }
      break;
    case R:
      if (reduce(&stack) != 0) {
        stack_free(&stack);
        return 2;
      }
      break;
    case E:
      push_stack(&stack, SYMBOL_TERMINAL, b);
      if (move_next_token(token) == 1) {
        stack_free(&stack);
        return 1;
      }
      break;
    case X:
      stack_free(&stack);
      return 2;

      break;
    }
  }
  if (stack.top != NULL && stack.top->type == SYMBOL_NONTERMINAL &&
      stack.top->next != NULL && stack.top->next->type == SYMBOL_TERMINAL &&
      stack.top->next->operator== DOLLAR && stack.top->next->next == NULL) {
    stack_free(&stack);
    return 0;
  } else {
    stack_free(&stack);
    return 2;
  }
}
