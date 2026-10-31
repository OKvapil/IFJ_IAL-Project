#include "scanner.h"
#include "token.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum { SYMBOL_TERMINAL, SYMBOL_NONTERMINAL, SYMBOL_STOP } SymbolType;

typedef struct StackNode {
  SymbolType type;
  struct StackNode *next;
} StackNode;

typedef struct {
  StackNode *top;
} Stack;

static void init_stack(Stack *s) { s->top = NULL; }

static void push_stack(Stack *s, SymbolType Type) {
  StackNode *new_node = malloc(sizeof(StackNode));
  if (new_node == NULL) {
    return;
  }

  new_node->type = Type;
  new_node->next = s->top;
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
  while (s->top != NULL) {
    if (s->top->type == SYMBOL_TERMINAL) {
      return s->top->type;
    } else {
      s->top->type = s->top->next;
    }
  }
}

static stack_push_after_top_terminal(Stack *s) {
  if (s->top == NULL)
    return;
  StackNode *stop_node = malloc(sizeof(StackNode));
  if (stop_node == NULL)
    return;

  if (s->top->type == SYMBOL_TERMINAL) {
    stop_node->next = s->top;
    s->top = stop_node;
  } else {
    stop_node->next = s->top->next;
    s->top->next = stop_node;
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
  s->top == NULL;
}

typedef enum {
  S, // Shift <
  R, // Reduce >
  E, // Equal =
  X  // Syntax error
} Operations;

typedef enum {
  PLUS,
  MINUS,
  MULT,
  DIV,
  LPAREN,
  RPAREN,
  ID,
  DOLLAR,
  TAB_MAX
} OperatorsTable;

static const precedence_table[TAB_MAX][TAB_MAX] = {
    //       +  -  *  /  r  e  (  )  ,  $  i
    /* + */ {R, R, S, S, R, R, S, R, R, R, S},
    /* - */ {R, R, S, S, R, R, S, R, R, R, S},
    /* * */ {R, R, R, R, R, R, S, R, R, R, S},
    /* / */ {R, R, R, R, R, R, S, R, R, R, S},
    /* r */ {S, S, S, S, X, X, S, R, R, X, S},
    /* e */ {S, S, S, S, X, X, S, R, R, X, S},
    /* ( */ {S, S, S, S, S, S, S, E, S, X, S},
    /* ) */ {R, R, R, R, R, R, X, R, R, R, X},
    /* i */ {R, R, R, R, R, R, E, R, R, R, X},
    /* , */ {S, S, S, S, S, S, S, R, R, X, S},
    /* $ */ {S, S, S, S, S, S, S, X, X, X, S}};

// e = equality operators
// r = relation operators
