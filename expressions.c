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
  StackNode *new_node = malloc(sizeof(SymbolType));
  if(new_node == NULL){
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

