/***********************************************************
* File: parser.h
* This file contains the parser for the interpreter.
* The parser is responsible for parsing the tokens from the lexer. Specifically, it constructs the Abstract Syntax Tree (AST).
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/




#pragma once

#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "lexer.h"
#include "ast.h"


typedef struct {
    TokenArray* tokens;
    size_t position;
} Parser;

// Main entry points
Parser create_parser(TokenArray* tokens);
ASTNode* parse_program(Parser* parser);
void parser_error(Parser* parser, const char* message);

// Statement parsing
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_block(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_while_statement(Parser* parser);
ASTNode* parse_for_statement(Parser* parser);
ASTNode* parse_array_list(Parser* parser);
ASTNode* parse_array_access(Parser* parser, ASTNode* identifier);

// Extended statements
ASTNode* parse_var_declaration(Parser* parser, int is_array);
ASTNode* parse_function_declaration(Parser* parser);


// Expression parsing
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_binary(Parser* parser, int precedence);
ASTNode* parse_unary(Parser* parser);
ASTNode* parse_postfix(Parser* parser);
ASTNode* parse_primary(Parser* parser);
ASTNode* parse_array_index(Parser* parser);
void parse_function_params(Parser* parser, ASTNode* funcNode);
long parse_binary_string(const char* binStr);
ASTNode* parse_switch_statement(Parser* parser);

// Utility
Token peek_token(Parser* parser);
Token consume_token(Parser* parser);
int match_token(Parser* parser, TokenKind type);
int get_precedence(TokenKind type);
void synchronize_to_next_case(Parser* parser);
void parse_case_statements(Parser* parser, ASTNode* caseNode);


#endif PARSER_H
