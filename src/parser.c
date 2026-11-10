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
#include "ast.h"
#include "expressions.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static Token current_token;

static AstNode *parse_program(void);

static void parse_import(void);

static AstNode *parse_statement(void);

static AstNode *parse_block(void);

static DataType parse_type(void);

static AstNode *parse_func_def(bool is_main);

static AstNode *parse_return_state(void);

static AstNode *parse_if_state(void);

static AstNode *parse_while_do_state(void);

static AstNode *parse_params(void);

static AstNode *parse_func_main(void);

void check_token(TokenType expected_token) {
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

static char *consume_identifier(void) {
    if (current_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Syntax error: expected identifier at line %zu\n", current_token.line);
        exit(2);
    }
    char *name = current_token.attr.str_val;
    current_token.attr.str_val = NULL;
    check_token(TOKEN_IDENTIFIER);
    return name;
}

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
                exit(2);
        }
    }
    return program;
}

static void parse_import(void) {
    check_token(TOKEN_KW_IMPORT);
    check_token(TOKEN_IDENTIFIER);
    check_token(TOKEN_DOT);
    check_token(TOKEN_MUL);
}

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
            AstNode *init_expr = parse_expression(&current_token);
            return ast_var_decl(kind, name, type, init_expr, line);
        }
        case TOKEN_KW_IF:
            return parse_if_state();
        case TOKEN_KW_WHILE:
            return parse_while_do_state();
        case TOKEN_KW_RETURN:
            return parse_return_state();
        case TOKEN_IDENTIFIER: {
            char *name = consume_identifier();
            check_token(TOKEN_ASSIGN);
            AstNode *value = parse_expression(&current_token);
            return ast_expr_assign(name, value, line);
        }
        default:
            fprintf(stderr, "Syntax error: expected type at line %zu\n", current_token.line);
            exit(2);
    }
}

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
        }
    }
    check_token(TOKEN_DEDENT);
    return block;
}

static DataType parse_type(void) {
    switch (current_token.type) {
        case TOKEN_KW_INT:
            check_token(TOKEN_KW_INT);
            return TYPE_INT;
            break;
        case TOKEN_KW_STRING:
            check_token(TOKEN_KW_STRING);
            return TYPE_STRING;
            break;
        case TOKEN_KW_DOUBLE:
            check_token(TOKEN_KW_DOUBLE);
            return TYPE_DOUBLE;
            break;
        case TOKEN_KW_UNIT:
            check_token(TOKEN_KW_UNIT);
            return TYPE_UNIT;
        default:
            fprintf(stderr, "Syntax error: expected type at line %zu\n", current_token.line);
            exit(2);
    }
}

static AstNode *parse_func_def(bool is_main) {
    size_t line = current_token.line;
    check_token(TOKEN_KW_DEF);
    char *name = consume_identifier();
    check_token(TOKEN_LPAREN);
    AstNode *params = parse_params();
    check_token(TOKEN_RPAREN);
    check_token(TOKEN_COLON);
    DataType return_type = parse_type();
    check_token(TOKEN_ASSIGN);

    AstNode *block = NULL;
    if (current_token.type == TOKEN_EOL) {
        block = parse_block();
    } else {
        block = parse_expression(&current_token);
    }

    size_t param_count = (params != NULL) ? params->data.block.count : 0;
    return ast_func_def(name, return_type, params, param_count, block, is_main, line);
}

static AstNode *parse_return_state(void) {
    size_t line = current_token.line;
    check_token(TOKEN_KW_RETURN);
    if (current_token.type != TOKEN_EOL && current_token.type != TOKEN_DEDENT) {
        AstNode *expr = parse_expression(&current_token);
        return ast_return(expr, line);
    }
    return ast_return(NULL, line);
}

static AstNode *parse_if_state(void) {
    size_t line = current_token.line;
    check_token(TOKEN_KW_IF);
    AstNode *cond = parse_expression(&current_token);
    check_token(TOKEN_KW_THEN);
    AstNode *then_block = parse_block();
    check_token(TOKEN_KW_ELSE);
    AstNode *else_block = parse_block();
    return ast_if(cond, then_block, else_block, line);
}

static AstNode *parse_while_do_state(void) {
    size_t line = current_token.line;
    check_token(TOKEN_KW_WHILE);
    AstNode *cond = parse_expression(&current_token);
    check_token(TOKEN_KW_DO);
    AstNode *block = parse_block();
    return ast_while(cond, block, line);
}

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
        AstNode *param = ast_var_decl(AST_VAL, param_name, param_type,NULL, line);
        ast_block_add_stmt(params_block, param);

        if (current_token.type == TOKEN_COMMA) {
            check_token(TOKEN_COMMA);
        } else {
            break;
        }
    }
    return params_block;
}

static AstNode *parse_func_main(void) {
    check_token(TOKEN_ANNOT_MAIN);
    while (current_token.type == TOKEN_EOL) {
        check_token(TOKEN_EOL);
    }
    return parse_func_def(true);
}
