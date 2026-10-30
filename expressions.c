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