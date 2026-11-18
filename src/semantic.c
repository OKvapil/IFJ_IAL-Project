/**
 * @file semantic.c
 * @brief The implementation of the semantic analysis.
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Jonáš Ľalík (xlalikj00)
 */

#include "semantic.h"
#include "symtable.h"
#include <stdlib.h>
#include <string.h>

static bool is_numeric(DataType type) {
  if (type == TYPE_INT || type == TYPE_DOUBLE) {
    return true;
  }
  return false;
}

static DataType binary_result_type(DataType op1_type, DataType op2_type) {
  if (!is_numeric(op1_type) || !is_numeric(op2_type)) {
    return TYPE_UNKNOWN;
  } else if (op1_type == TYPE_DOUBLE || op2_type == TYPE_DOUBLE) {
    return TYPE_DOUBLE;
  } else {
    return TYPE_INT;
  }
}

static bool is_assignable(DataType decl_type, DataType init_type) {
  if (init_type == TYPE_UNKNOWN) {
    return false;
  }
  if ((decl_type == TYPE_DOUBLE && init_type == TYPE_INT) ||
      (decl_type == init_type) || (decl_type == TYPE_ANY)) {
    return true;
  }
  return false;
}

static Symbol *current_function;
static AstNode *program_root;
static unsigned char *infer_state_flags;

static bool always_returns(AstNode *node) {
  if (node == NULL) {
    return false;
  }

  switch (node->kind) {
  case AST_RETURN:
    return true;

  case AST_BLOCK:
    if (node->data.block.count > 0 &&
        always_returns(
            node->data.block.statements[node->data.block.count - 1])) {
      return true;
    }
    return false;

  case AST_IF:
    if (node->data.if_state.else_block != NULL &&
        always_returns(node->data.if_state.then_block) &&
        always_returns(node->data.if_state.else_block)) {
      return true;
    }
    return false;

  default:
    return false;
  }
}

static size_t get_function_index(const char *name) {
  for (size_t i = 0; i < program_root->data.block.count; i++) {
    AstNode *def = program_root->data.block.statements[i];
    if (strcmp(def->data.func_def.name, name) == 0) {
      return i;
    }
  }
  return (size_t)-1;
}

static ErrorCode analyze_node(AstNode *node, SymTable *symtable) {
  switch (node->kind) {
  case AST_INT_LIT:
    node->result_type = TYPE_INT;
    break;

  case AST_DOUBLE_LIT:
    node->result_type = TYPE_DOUBLE;
    break;

  case AST_STRING_LIT:
    node->result_type = TYPE_STRING;
    break;

  case AST_IDENTIFIER: {
    Symbol *symbol = st_lookup(symtable, node->data.text);
    if (symbol == NULL || symbol->kind != SYMBOL_VARIABLE) {
      return ERR_UNDEFINED;
    }
    node->result_type = symbol->type;
    node->var_id = symbol->var_unique_id;
    break;
  }

  case AST_UNARY: {
    TokenType operation = node->data.unary.op;
    AstNode *operand = node->data.unary.operand;

    ErrorCode analyze_operand = analyze_node(operand, symtable);
    if (analyze_operand != ERR_OK) {
      return analyze_operand;
    }

    if (operation == TOKEN_MINUS || operation == TOKEN_PLUS) {
      if (operand->result_type != TYPE_INT &&
          operand->result_type != TYPE_DOUBLE) {
        return ERR_TYPE;
      }
      node->result_type = node->data.unary.operand->result_type;
    }

    else if (operation == TOKEN_NOT) {
      if (operand->result_type != TYPE_BOOL) {
        return ERR_TYPE;
      }
      node->result_type = TYPE_BOOL;
    }

    else {
      return ERR_INTERNAL;
    }
    break;
  }

  case AST_BINARY: {
    TokenType operation = node->data.binary.op;
    AstNode *operand1 = node->data.binary.left;
    AstNode *operand2 = node->data.binary.right;

    ErrorCode analyze_operand1 = analyze_node(operand1, symtable);
    if (analyze_operand1 != ERR_OK) {
      return analyze_operand1;
    }

    ErrorCode analyze_operand2 = analyze_node(operand2, symtable);
    if (analyze_operand2 != ERR_OK) {
      return analyze_operand2;
    }

    /* AI-generated (Claude). Prompt: "rewrite this chain of ifs into the switch
    case" (binary arithmetic operators). */
    switch (operation) {
    case TOKEN_PLUS:
      if (operand1->result_type == TYPE_STRING &&
          operand2->result_type == TYPE_STRING) {
        node->result_type = TYPE_STRING;
        break;
      }

    /* fall through */
    case TOKEN_MINUS:
    case TOKEN_MUL:
    case TOKEN_DIV: {
      DataType result_type =
          binary_result_type(operand1->result_type, operand2->result_type);
      if (result_type == TYPE_UNKNOWN) {
        return ERR_TYPE;
      }
      node->result_type = result_type;
      break;
    }

    case TOKEN_LT:
    case TOKEN_LTE:
    case TOKEN_GT:
    case TOKEN_GTE: {
      if ((is_numeric(operand1->result_type) &&
           is_numeric(operand2->result_type)) ||
          (operand1->result_type == TYPE_STRING &&
           operand2->result_type == TYPE_STRING)) {
        node->result_type = TYPE_BOOL;
        break;
      } else {
        return ERR_TYPE;
      }
    }

    case TOKEN_EQ:
    case TOKEN_NEQ: {
      if (operand1->result_type == TYPE_UNKNOWN ||
          operand2->result_type == TYPE_UNKNOWN) {
        return ERR_TYPE;
      }
      if ((is_numeric(operand1->result_type) &&
           is_numeric(operand2->result_type)) ||
          operand1->result_type == operand2->result_type) {
        node->result_type = TYPE_BOOL;
        break;
      } else {
        return ERR_TYPE;
      }
    }

    case TOKEN_AND:
    case TOKEN_OR: {
      if (operand1->result_type != TYPE_BOOL ||
          operand2->result_type != TYPE_BOOL) {
        return ERR_TYPE;
      }
      node->result_type = TYPE_BOOL;
      break;
    }

    default:
      return ERR_INTERNAL;
    }

    break;
  }

  case AST_FUNC_DEF: {
    Symbol *symbol = st_lookup(symtable, node->data.func_def.name);
    if (symbol == NULL) {
      return ERR_INTERNAL;
    }

    size_t function_index = get_function_index(node->data.func_def.name);
    if (function_index == (size_t)-1) {
      return ERR_INTERNAL;
    }
    if (infer_state_flags[function_index] == 2) {
      return ERR_OK;
    }
    infer_state_flags[function_index] = 1;

    if (st_enter_scope(symtable) == -1) {
      return ERR_INTERNAL;
    }

    for (size_t i = 0; i < node->data.func_def.param_count; i++) {
      Symbol *param_symbol = NULL;
      AstNode *param = node->data.func_def.params[i];
      DeclarationCode declared_param =
          st_declare_var(symtable, param->data.var_dec.name,
                         param->data.var_dec.var_type, false, &param_symbol);
      if (declared_param == DECL_REDEF) {
        return ERR_REDEFINITION;
      } else if (declared_param == DECL_ALLOC_FAIL) {
        return ERR_INTERNAL;
      }
      param->var_id = param_symbol->var_unique_id;
    }

    Symbol *saved = current_function;
    current_function = symbol;

    if (node->data.func_def.block != NULL) {
      ErrorCode analyze_block =
          analyze_node(node->data.func_def.block, symtable);
      if (analyze_block != ERR_OK) {
        return analyze_block;
      }
    }

    current_function = saved;
    st_exit_scope(symtable);

    bool ends_with_return = always_returns(node->data.func_def.block);

    if (ends_with_return == false) {
      DataType body_type = node->data.func_def.block->result_type;
      if (symbol->type == TYPE_UNKNOWN) {
        if (body_type == TYPE_UNKNOWN) {
          return ERR_TYPE;
        }
        symbol->type = body_type;
        node->data.func_def.return_type = body_type;
        node->result_type = body_type;
      } else if (symbol->type != TYPE_UNIT) {
        if (!is_assignable(symbol->type, body_type)) {
          return ERR_TYPE;
        }
      }
    }

    infer_state_flags[function_index] = 2;

    break;
  }

  case AST_FUNC_CALL: {
    Symbol *symbol = st_lookup(symtable, node->data.func_call.name);
    if (symbol == NULL || symbol->kind != SYMBOL_FUNCTION) {
      return ERR_UNDEFINED;
    }

    if (node->data.func_call.arg_count != (size_t)symbol->param_count) {
      return ERR_CALL_ARGS;
    }

    for (size_t i = 0; i < node->data.func_call.arg_count; i++) {
      ErrorCode analyze_argument =
          analyze_node(node->data.func_call.args[i], symtable);
      if (analyze_argument != ERR_OK) {
        return analyze_argument;
      }
      if (!(is_assignable(symbol->param_types[i],
                          node->data.func_call.args[i]->result_type))) {
        return ERR_CALL_ARGS;
      }
    }

    if (symbol->type == TYPE_UNKNOWN) {
      size_t function_index = get_function_index(node->data.func_call.name);
      if (function_index == (size_t)-1) {
        return ERR_INTERNAL;
      }
      if (infer_state_flags[function_index] == 1) {
        return ERR_TYPE;
      } else if (infer_state_flags[function_index] == 0) {
        SymScope *current_scope = symtable->current_scope;
        SymScope *global_scope = current_scope;
        while (global_scope->outer != NULL) {
          global_scope = global_scope->outer;
        }

        symtable->current_scope = global_scope;

        ErrorCode analyze_function = analyze_node(
            program_root->data.block.statements[function_index], symtable);

        while (symtable->current_scope != global_scope) {
          st_exit_scope(symtable);
        }
        symtable->current_scope = current_scope;

        if (analyze_function != ERR_OK) {
          return analyze_function;
        }
      }
    }
    if (symbol->type == TYPE_UNKNOWN) {
      return ERR_TYPE;
    }

    node->result_type = symbol->type;
    break;
  }

  case AST_RETURN: {
    if (current_function->type == TYPE_UNKNOWN) {
      return ERR_TYPE;
    }

    DataType value_type = TYPE_UNIT;
    if (node->data.return_state.expression != NULL) {
      ErrorCode analyze_expression =
          analyze_node(node->data.return_state.expression, symtable);
      if (analyze_expression != ERR_OK) {
        return analyze_expression;
      }
      value_type = node->data.return_state.expression->result_type;
    }

    if (node->data.return_state.expression == NULL &&
        current_function->type != TYPE_UNIT) {
      return ERR_OTHER_SEMANTIC;
    }

    if (current_function->type != TYPE_UNIT) {
      if (!(is_assignable(current_function->type, value_type))) {
        return ERR_TYPE;
      }
    }

    node->result_type = TYPE_UNIT;
    break;
  }

  case AST_IF: {
    ErrorCode analyze_condition =
        analyze_node(node->data.if_state.cond, symtable);
    if (analyze_condition != ERR_OK) {
      return analyze_condition;
    }

    if (node->data.if_state.cond->result_type != TYPE_BOOL) {
      return ERR_TYPE;
    }

    ErrorCode analyze_then =
        analyze_node(node->data.if_state.then_block, symtable);
    if (analyze_then != ERR_OK) {
      return analyze_then;
    }

    if (node->data.if_state.else_block != NULL) {
      ErrorCode analyze_else =
          analyze_node(node->data.if_state.else_block, symtable);
      if (analyze_else != ERR_OK) {
        return analyze_else;
      }
    }

    DataType then_type = node->data.if_state.then_block->result_type;
    DataType else_type;
    if (node->data.if_state.else_block == NULL) {
      else_type = TYPE_UNIT;
    } else {
      else_type = node->data.if_state.else_block->result_type;
    }

    if (always_returns(node->data.if_state.then_block) &&
        always_returns(node->data.if_state.else_block)) {
      node->result_type = TYPE_UNIT;
    } else if (always_returns(node->data.if_state.then_block) &&
               !always_returns(node->data.if_state.else_block)) {
      node->result_type = else_type;
    } else if (!always_returns(node->data.if_state.then_block) &&
               always_returns(node->data.if_state.else_block)) {
      node->result_type = then_type;
    } else {
      if (then_type == else_type) {
        node->result_type = then_type;
      } else if (binary_result_type(then_type, else_type) == TYPE_DOUBLE) {
        node->result_type = TYPE_DOUBLE;
      } else {
        node->result_type = TYPE_UNKNOWN;
      }
    }

    break;
  }

  case AST_WHILE: {
    ErrorCode analyze_condition =
        analyze_node(node->data.while_state.cond, symtable);
    if (analyze_condition != ERR_OK) {
      return analyze_condition;
    }

    if (node->data.while_state.cond->result_type != TYPE_BOOL) {
      return ERR_TYPE;
    }

    ErrorCode analyze_body =
        analyze_node(node->data.while_state.block, symtable);
    if (analyze_body != ERR_OK) {
      return analyze_body;
    }

    node->result_type = TYPE_UNIT;
    break;
  }

  case AST_BLOCK: {
    if (st_enter_scope(symtable) == -1) {
      return ERR_INTERNAL;
    }

    for (size_t i = 0; i < node->data.block.count; i++) {
      ErrorCode node_result_error =
          analyze_node(node->data.block.statements[i], symtable);
      if (node_result_error != ERR_OK) {
        return node_result_error;
      }
    }
    DataType block_type = TYPE_UNIT;
    if (node->data.block.count > 0) {
      block_type =
          node->data.block.statements[node->data.block.count - 1]->result_type;
    }
    node->result_type = block_type;
    st_exit_scope(symtable);
    break;
  }

  case AST_ASSIGN: {
    Symbol *symbol = st_lookup(symtable, node->data.expr_assign.name);
    if (symbol == NULL) {
      return ERR_UNDEFINED;
    }

    if (symbol->var_modifiable == false) {
      return ERR_IMMUTABLE;
    }

    ErrorCode analyze_value =
        analyze_node(node->data.expr_assign.value, symtable);
    if (analyze_value != ERR_OK) {
      return analyze_value;
    }

    if (!(is_assignable(symbol->type,
                        node->data.expr_assign.value->result_type))) {
      return ERR_TYPE;
    }

    node->var_id = symbol->var_unique_id;
    node->result_type = TYPE_UNIT;

    break;
  }

  case AST_VAR:
  case AST_VAL: {
    if (node->data.var_dec.init_expr == NULL) {
      return ERR_INTERNAL;
    }

    ErrorCode init_result =
        analyze_node(node->data.var_dec.init_expr, symtable);
    if (init_result != ERR_OK) {
      return init_result;
    }

    if (node->data.var_dec.var_type != TYPE_UNKNOWN) {
      if (!(is_assignable(node->data.var_dec.var_type,
                          node->data.var_dec.init_expr->result_type))) {
        return ERR_TYPE;
      }
    } else {
      if (node->data.var_dec.init_expr->result_type == TYPE_UNKNOWN) {
        return ERR_TYPE;
      } else {
        node->data.var_dec.var_type = node->data.var_dec.init_expr->result_type;
      }
    }

    Symbol *symbol = NULL;
    DeclarationCode decl_result = st_declare_var(
        symtable, node->data.var_dec.name, node->data.var_dec.var_type,
        node->kind == AST_VAR, &symbol);
    if (decl_result == DECL_REDEF) {
      return ERR_REDEFINITION;
    } else if (decl_result == DECL_ALLOC_FAIL) {
      return ERR_INTERNAL;
    }

    node->var_id = symbol->var_unique_id;

    node->result_type = TYPE_UNIT;

    break;
  }

  case AST_BOOL_LIT:
    node->result_type = TYPE_BOOL;
    break;

  case AST_UNIT_LIT:
    node->result_type = TYPE_UNIT;
    break;

  case AST_FOR: {
    ErrorCode analyze_from = analyze_node(node->data.for_state.from, symtable);
    if (analyze_from != ERR_OK) {
      return analyze_from;
    }

    ErrorCode analyze_to = analyze_node(node->data.for_state.to, symtable);
    if (analyze_to != ERR_OK) {
      return analyze_to;
    }

    if (node->data.for_state.step != NULL) {
      ErrorCode analyze_step =
          analyze_node(node->data.for_state.step, symtable);
      if (analyze_step != ERR_OK) {
        return analyze_step;
      }
    }

    if (node->data.for_state.from->result_type != TYPE_INT ||
        node->data.for_state.to->result_type != TYPE_INT) {
      return ERR_TYPE;
    }

    if (node->data.for_state.step != NULL) {
      if (node->data.for_state.step->result_type != TYPE_INT) {
        return ERR_TYPE;
      }

      if (node->data.for_state.step->kind == AST_INT_LIT &&
          node->data.for_state.step->data.int_value == 0) {
        return ERR_OTHER_SEMANTIC;
      }
    }

    if (st_enter_scope(symtable) == -1) {
      return ERR_INTERNAL;
    }

    Symbol *symbol = NULL;
    DeclarationCode declare_var = st_declare_var(
        symtable, node->data.for_state.var_name, TYPE_INT, false, &symbol);
    if (declare_var == DECL_REDEF) {
      return ERR_REDEFINITION;
    } else if (declare_var == DECL_ALLOC_FAIL) {
      return ERR_INTERNAL;
    }
    node->var_id = symbol->var_unique_id;

    for (size_t i = 0; i < node->data.for_state.guard_count; i++) {
      ErrorCode analyze_guard =
          analyze_node(node->data.for_state.guards[i], symtable);
      if (analyze_guard != ERR_OK) {
        return analyze_guard;
      }
      if (node->data.for_state.guards[i]->result_type != TYPE_BOOL) {
        return ERR_TYPE;
      }
    }

    ErrorCode analyze_body = analyze_node(node->data.for_state.body, symtable);
    if (analyze_body != ERR_OK) {
      return analyze_body;
    }

    st_exit_scope(symtable);
    node->result_type = TYPE_UNIT;
    break;
  }
  }

  return ERR_OK;
} // analyze_node

static bool check_indirect_recursive(AstNode *node, size_t target_idx, size_t *visited) {
  if (node == NULL) {
    return false;
  }
  
  switch (node->kind) {
    case AST_BLOCK:
      for (size_t i = 0; i < node->data.block.count; i++) {
        if (check_indirect_recursive(node->data.block.statements[i], target_idx, visited)) {
          return true;
        }
      }
      break;

    case AST_FUNC_CALL: {
      for (size_t i = 0; i < node->data.func_call.arg_count; i++) {
        if (check_indirect_recursive(node->data.func_call.args[i], target_idx, visited)) {
          return true;
        }
      }

      size_t idx = get_function_index(node->data.func_call.name);
      if (idx == (size_t)-1) {
        return false;
      }

      if (idx == target_idx) {
        return true;
      }

      if (visited[idx] == 0) {
        visited[idx] = 1;
        if (check_indirect_recursive(program_root->data.block.statements[idx]->data.func_def.block, target_idx, visited)) {
          return true;
        }
      }

      break;
    }

    case AST_FOR:
      for (size_t i = 0; i < node->data.for_state.guard_count; i++) {
        if (check_indirect_recursive(node->data.for_state.guards[i], target_idx, visited)) {
          return true;
        }
      }
      
      if (check_indirect_recursive(node->data.for_state.from, target_idx, visited) || 
          check_indirect_recursive(node->data.for_state.to, target_idx, visited) ||
          check_indirect_recursive(node->data.for_state.step, target_idx, visited) ||
          check_indirect_recursive(node->data.for_state.body, target_idx, visited)) {
        return true;
      }

      break;

    case AST_UNARY:
      if (check_indirect_recursive(node->data.unary.operand, target_idx, visited)) {
        return true;
      }
      break;

    case AST_BINARY:
      if (check_indirect_recursive(node->data.binary.left, target_idx, visited) || check_indirect_recursive(node->data.binary.right, target_idx, visited)) {
        return true;
      }
      break;

    case AST_RETURN:
      if (check_indirect_recursive(node->data.return_state.expression, target_idx, visited)) {
        return true;
      }
      break;

    case AST_IF:
      if (check_indirect_recursive(node->data.if_state.cond, target_idx, visited) || 
          check_indirect_recursive(node->data.if_state.then_block, target_idx, visited) || 
          check_indirect_recursive(node->data.if_state.else_block, target_idx, visited)) {
        return true;
      }
      break;

    case AST_WHILE:
      if (check_indirect_recursive(node->data.while_state.cond, target_idx, visited) || check_indirect_recursive(node->data.while_state.block, target_idx, visited)) {
        return true;
      }
      break;

    case AST_ASSIGN:
      if (check_indirect_recursive(node->data.expr_assign.value, target_idx, visited)) {
        return true;
      }
      break;

    case AST_VAL:
      if (check_indirect_recursive(node->data.var_dec.init_expr, target_idx, visited)) {
        return true;
      }
      break;

    case AST_VAR:
      if (check_indirect_recursive(node->data.var_dec.init_expr, target_idx, visited)) {
        return true;
      }
      break;

    case AST_INT_LIT:
    case AST_DOUBLE_LIT:
    case AST_STRING_LIT:
    case AST_BOOL_LIT:
    case AST_UNIT_LIT:
    case AST_IDENTIFIER:
    case AST_FUNC_DEF:
      return false;
  }
  return false;

}

static ErrorCode find_indirect_recursive(AstNode *root) {
  size_t *visited = calloc(root->data.block.count + 1, sizeof(size_t));
  if (visited == NULL) {
    return ERR_INTERNAL;
  }

  for (size_t i = 0; i < root->data.block.count; i++) {
    if (root->data.block.statements[i]->data.func_def.return_type == TYPE_UNKNOWN) {
      for (size_t j = 0; j < root->data.block.count + 1; j++) {
        visited[j] = 0;
      }
      if (check_indirect_recursive(program_root->data.block.statements[i]->data.func_def.block, i, visited)) {
        free(visited);
        return ERR_TYPE;
      }
    }
  }

  free(visited);
  return ERR_OK;
}

static ErrorCode analyze_program(AstNode *root, SymTable *symtable) {
  size_t main_count = 0;
  AstNode *main_def = NULL;

  for (size_t i = 0; i < root->data.block.count; i++) {
    AstNode *def = root->data.block.statements[i];
    if (def->kind != AST_FUNC_DEF) {
      return ERR_INTERNAL;
    }

    /* AI-generated (Claude). Prompt: "Generate code for the arrays" for the
    signature pass in sem_analyze: temporary type and name arrays. */
    size_t count = def->data.func_def.param_count;
    DataType *types = NULL;
    const char **names = NULL;
    if (count > 0) {
      types = malloc(count * sizeof(DataType));
      names = malloc(count * sizeof(char *));
      if (types == NULL || names == NULL) {
        free(types);
        free(names);
        return ERR_INTERNAL;
      }
      for (size_t j = 0; j < count; j++) {
        AstNode *param = def->data.func_def.params[j];
        types[j] = param->data.var_dec.var_type;
        names[j] = param->data.var_dec.name;
      }
    }

    DeclarationCode decl_fun = st_declare_fun(symtable, def->data.func_def.name,
                                              def->data.func_def.return_type,
                                              (int)count, types, names, NULL);
    free(types);
    free(names);
    if (decl_fun == DECL_REDEF) {
      return ERR_REDEFINITION;
    } else if (decl_fun == DECL_ALLOC_FAIL) {
      return ERR_INTERNAL;
    }

    if (def->data.func_def.is_main) {
      main_count++;
      main_def = def;
    }
  }

  if (main_count == 0) {
    return ERR_UNDEFINED;
  } else if (main_count > 1) {
    return ERR_REDEFINITION;
  }

  if (main_def->data.func_def.param_count != 0) {
    return ERR_CALL_ARGS;
  } else if (main_def->data.func_def.return_type != TYPE_UNIT &&
             main_def->data.func_def.return_type != TYPE_UNKNOWN) {
    return ERR_TYPE;
  }

  ErrorCode indirect_recursive = find_indirect_recursive(program_root);
  if (indirect_recursive != ERR_OK) {
    return indirect_recursive;
  }

  current_function = NULL;
  for (size_t i = 0; i < root->data.block.count; i++) {
    ErrorCode result = analyze_node(root->data.block.statements[i], symtable);
    if (result != ERR_OK) {
      return result;
    }
  }

  Symbol *symbol = st_lookup(symtable, main_def->data.func_def.name);
  if (symbol == NULL) {
    return ERR_INTERNAL;
  }
  if (symbol->type != TYPE_UNIT) {
    return ERR_TYPE;
  }

  return ERR_OK;
}

ErrorCode sem_analyze(AstNode *root) {
  SymTable *symtable = st_init();
  if (symtable == NULL) {
    return ERR_INTERNAL;
  }
  program_root = root;

  infer_state_flags = calloc(root->data.block.count + 1, sizeof(char));
  if (infer_state_flags == NULL) {
    st_dispose(symtable);
    program_root = NULL;
    return ERR_INTERNAL;
  }

  ErrorCode analyzed_program = analyze_program(root, symtable);

  st_dispose(symtable);
  current_function = NULL;
  free(infer_state_flags);
  infer_state_flags = NULL;
  program_root = NULL;

  return analyzed_program;
}