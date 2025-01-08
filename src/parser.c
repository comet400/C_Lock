/***********************************************************
* File: parser.c
* This file contains the parser for the interpreter.
* The parser is responsible for parsing the tokens from the lexer. Specifically, it constructs the Abstract Syntax Tree (AST).
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "ast.h"



/***********************************************************
* Function: create_parser
* Description: this function creates the parser
* Parameters: TokenArray* tokens
* Return: Parser
* ***********************************************************/
Parser create_parser(TokenArray* tokens) {
    Parser parser;
    parser.tokens = tokens;
    parser.position = 0;
    return parser;
}






/***********************************************************
* Function: peek_token
* Description: this function peeks the token
* Parameters: Parser* parser
* Return: Token
* ***********************************************************/
Token peek_token(Parser* parser) {
    if (parser->position < parser->tokens->size) {
        return parser->tokens->data[parser->position];
    }
    return make_token("", TOKEN_EOF); // fallback
}






/***********************************************************
* Function: consume_token
* Description: this function consumes the token
* Parameters: Parser* parser
* Return: Token
* ***********************************************************/
Token consume_token(Parser* parser) {
    Token t = peek_token(parser);
    if (parser->position < parser->tokens->size) {
        parser->position++;
    }
    return t;
}






/***********************************************************
* Function: match_token
* Description: this function matches the token
* Parameters: Parser* parser, TokenKind type
* Return: int
* ***********************************************************/
int match_token(Parser* parser, TokenKind type) {
    Token current = peek_token(parser);
    if (current.type == type) {
        consume_token(parser);
        return 1;
    }
    return 0;
}






/***********************************************************
* Function: get_precedence
* Description: this function gets the precedence
* Parameters: TokenKind type
* Return: int
* ***********************************************************/
void parser_error(Parser* parser, const char* message) {
    Token t = peek_token(parser);
    fprintf(stderr,
        "Parse error at line %zu, col %zu: %s (got token type=%d, value='%s')\n",
        t.line, t.column,
        message,
        t.type,
        t.value ? t.value : "");
    exit(EXIT_FAILURE);
}






/***********************************************************
* Function: parse_program
* Description: this function parses the program
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_program(Parser* parser) {
    Token t = peek_token(parser);
    /* Create a PROGRAM node at line/col of the first token */
    ASTNode* root = create_ast_node(AST_PROGRAM, t.line, t.column, NULL);

    while (peek_token(parser).type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            ast_add_child(root, stmt);
        }
    }
    return root;
}






/***********************************************************
* Function: parse_statement
* Description: this function parses the statement
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_statement(Parser* parser) {
    Token t = peek_token(parser);

    /* Skip empty statement if we see TOKEN_END (like a stray semicolon) */
    if (t.type == TOKEN_END) {
        consume_token(parser);
        return NULL;
    }

    switch (t.type) {
    case TOKEN_IF:
    case TOKEN_IFNOT:
        return parse_if_statement(parser);

    case TOKEN_WHILE:
    case TOKEN_WHILENOT:
        return parse_while_statement(parser);

    case TOKEN_FOR:
        return parse_for_statement(parser);

    case TOKEN_SWITCH:
        return parse_switch_statement(parser);

    case TOKEN_RETURN: {
        consume_token(parser); // consume 'return'
        Token returnTok = t;   // store line/col for the node

        ASTNode* retNode = create_ast_node(AST_RETURN_STATEMENT,
            returnTok.line, returnTok.column,
            NULL);

        /* If next token is not a block end, semicolon, or EOF, parse an expression */
        Token nxt = peek_token(parser);
        if (nxt.type != TOKEN_END &&
            nxt.type != TOKEN_ENDBLOCK &&
            nxt.type != TOKEN_EOF)
        {
            ASTNode* expr = parse_expression(parser);
            if (!expr) {
                parser_error(parser, "Expected expression after 'return'.");
            }
            ast_add_child(retNode, expr);
        }
        /* Expect a semicolon/end */
        if (!match_token(parser, TOKEN_END)) {
            parser_error(parser, "Expected ';' or 'end' after return statement.");
        }
        return retNode;
    }

    case TOKEN_BEGINBLOCK:
        return parse_block(parser);

	case TOKEN_VAR:     // safe var declaration requires an assignment
        return parse_var_declaration(parser, 0);

	case TOKEN_ARRAY:   // list declaration for arrays
        return parse_var_declaration(parser, 1);

	case TOKEN_FUNCTION: // function declaration
        return parse_function_declaration(parser);

    default: {
        /* this is the token that holds any identifier within the language */
        if (t.type == TOKEN_IDENTIFIER) {
            // Look ahead for assignment operators
            if ((parser->position + 1) < parser->tokens->size) {
                Token nextTok = parser->tokens->data[parser->position + 1];
                if (nextTok.type == TOKEN_EQUALS ||
                    nextTok.type == TOKEN_PLUS_EQUALS ||
                    nextTok.type == TOKEN_MINUS_EQUALS ||
                    nextTok.type == TOKEN_MULT_EQUALS ||
                    nextTok.type == TOKEN_DIV_EQUALS ||
                    nextTok.type == TOKEN_MOD_EQUALS)
                {
                    /* It's an assignment */
                    Token idTok = consume_token(parser); // identifier
                    Token opTok = consume_token(parser); // operator
                    ASTNode* rhs = parse_expression(parser);
                    if (!rhs) {
                        parser_error(parser, "Expected expression in assignment.");
                    }
                    ASTNode* assign = create_ast_node(AST_ASSIGNMENT,
                        idTok.line, idTok.column,
                        opTok.value);

                    /* Left side (identifier) as child 0 */
                    ASTNode* identNode = create_ast_node(AST_IDENTIFIER,
                        idTok.line, idTok.column,
                        idTok.value);
                    ast_add_child(assign, identNode);

                    /* Right side as child 1 */
                    ast_add_child(assign, rhs);

                    /* Expect semicolon/end */
                    if (!match_token(parser, TOKEN_END)) {
                        parser_error(parser, "Expected ';' after assignment.");
                    }
                    return assign;
                }
            }
        }
        /* Otherwise parse an expression statement */
        ASTNode* expr = parse_expression(parser);
        if (!expr) {
            parser_error(parser, "Invalid expression statement.");
        }
        if (!match_token(parser, TOKEN_END)) {
            parser_error(parser, "Missing ';' after expression statement.");
        }
        return expr;
    }
    }
}






/***********************************************************
* Function: parse_block
* Description: this function parses the block { }
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_block(Parser* parser) {
    Token start = peek_token(parser);
    if (!match_token(parser, TOKEN_BEGINBLOCK)) {
        parser_error(parser, "Expected '{' to begin block.");
    }

    ASTNode* block = create_ast_node(AST_BLOCK, start.line, start.column, NULL);

    while (1) {
        Token t = peek_token(parser);
        if (t.type == TOKEN_ENDBLOCK) {
            consume_token(parser); // consume '}'
            break;
        }
        if (t.type == TOKEN_EOF) {
            parser_error(parser, "Unexpected EOF in block; missing '}'?");
        }

        ASTNode* stmt = parse_statement(parser);
        if (!stmt) {
            parser_error(parser, "Invalid or empty statement in block.");
        }
        ast_add_child(block, stmt);
    }

    return block;
}






/***********************************************************
* Function: parse_if_statement
* Description: this function parses the if statement
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_if_statement(Parser* parser) {
    Token ifTok = peek_token(parser);

    int isNot = 0;
    if (match_token(parser, TOKEN_IFNOT)) {
        isNot = 1;
    }
    else if (!match_token(parser, TOKEN_IF)) {
        parser_error(parser, "Expected 'if' or 'ifNot'.");
    }

    if (!match_token(parser, TOKEN_BEGINPARAMS)) {
        parser_error(parser, "Expected '(' after 'if'.");
    }

    /* Using parse_expression for the condition (rather than parse_boolean) */
    ASTNode* condition = parse_expression(parser);
    if (!condition) {
        parser_error(parser, "Expected condition expression in if statement.");
    }
    /* ifNot => wrap condition in a unary '!' node I decided to not use it for the language this time */
    if (isNot) {
        ASTNode* notNode = create_ast_node(AST_UNARY_EXPR,
            ifTok.line, ifTok.column,
            "!");
        ast_add_child(notNode, condition);
        condition = notNode;
    }

    if (!match_token(parser, TOKEN_ENDPARAMS)) {
        parser_error(parser, "Expected ')' after 'if' condition.");
    }

    /* Parse the block that follows */
    ASTNode* thenBlock = parse_block(parser);
    if (!thenBlock) {
        parser_error(parser, "Missing block after 'if' condition.");
    }

    ASTNode* ifNode = create_ast_node(AST_IF_STATEMENT,
        ifTok.line, ifTok.column,
        NULL);
    ast_add_child(ifNode, condition);
    ast_add_child(ifNode, thenBlock);

    /* Optional else for the user */
    if (match_token(parser, TOKEN_ELSE)) {
        Token nxt = peek_token(parser);
        if (nxt.type == TOKEN_IF || nxt.type == TOKEN_IFNOT) {
            /* else if => parse_if_statement recursively */
            ASTNode* elseIfNode = parse_if_statement(parser);
            if (!elseIfNode) {
                parser_error(parser, "Invalid 'else if' block.");
            }
            ast_add_child(ifNode, elseIfNode);
        }
        else {
            /* normal else => parse a block */
            ASTNode* elseBlock = parse_block(parser);
            if (!elseBlock) {
                parser_error(parser, "Missing block after 'else'.");
            }
            ast_add_child(ifNode, elseBlock);
        }
    }

    return ifNode;
}






/***********************************************************
* Function: parse_while_statement
* Description: this function parses the while statement
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_while_statement(Parser* parser) {
    Token wTok = peek_token(parser);

    int isNot = 0;
    if (match_token(parser, TOKEN_WHILENOT)) {
        isNot = 1;
    }
    else if (!match_token(parser, TOKEN_WHILE)) {
        parser_error(parser, "Expected 'while' or 'whileNot'.");
    }

    if (!match_token(parser, TOKEN_BEGINPARAMS)) {
        parser_error(parser, "Expected '(' after 'while'.");
    }

    /* Just parse an expression for the condition, not a special parse_bool_expression */
    ASTNode* condition = parse_expression(parser);
    if (!condition) {
        parser_error(parser, "Invalid while condition.");
    }

    if (isNot) {
        ASTNode* notNode = create_ast_node(AST_UNARY_EXPR,
            wTok.line, wTok.column,
            "!");
        ast_add_child(notNode, condition);
        condition = notNode;
    }

    ASTNode* whileNode = create_ast_node(AST_WHILE_STATEMENT,
        wTok.line, wTok.column,
        NULL);
    ast_add_child(whileNode, condition);

    if (!match_token(parser, TOKEN_ENDPARAMS)) {
        parser_error(parser, "Expected ')' after 'while' condition.");
    }

    ASTNode* body = parse_block(parser);
    if (!body) {
        parser_error(parser, "Missing block after 'while(...)'.");
    }
    ast_add_child(whileNode, body);

    return whileNode;
}





/***********************************************************
* Function: synchronize_to_next_case
* Description: this is responsible for synchronizing the next case of the tokens below
* Parameters: Parser* parser
* Return: void
* ***********************************************************/
void synchronize_to_next_case(Parser* parser) {
    while (peek_token(parser).type != TOKEN_WHEN &&
        peek_token(parser).type != TOKEN_DEFAULT &&
        peek_token(parser).type != TOKEN_ENDBLOCK &&
        peek_token(parser).type != TOKEN_EOF) {
        consume_token(parser);
    }
}




/***********************************************************
* Function: parse_case_statements
* Description: this function parses the case statements
* Parameters: Parser* parser, ASTNode* caseNode
* Return: void
* ***********************************************************/
void parse_case_statements(Parser* parser, ASTNode* caseNode) {
    while (1) {
        Token stmtTok = peek_token(parser);

        // Check for 'stop' to terminate the case
        if (stmtTok.type == TOKEN_BREAK) {
            consume_token(parser); // Consume 'stop'
            if (match_token(parser, TOKEN_END)) {
                // Optional semicolon after 'stop'
            }
            break; // End the case block
        }

        // Handle unexpected EOF or end of block
        if (stmtTok.type == TOKEN_EOF || stmtTok.type == TOKEN_ENDBLOCK) {
            parser_error(parser, "Unexpected EOF or '}' in case; missing 'stop'?");
            break;
        }

        // Parse a regular statement
        ASTNode* stmt = parse_statement(parser);
        if (!stmt) {
            parser_error(parser, "Invalid or empty statement in case.");
            consume_token(parser); // Prevent infinite loops
            continue;
        }

        // Add the statement to the case node
        ast_add_child(caseNode, stmt);
    }
}




/***********************************************************
* Function: parse_switch_statement
* Description: this function parses the switch statement
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_switch_statement(Parser* parser) {
    Token switchTok = peek_token(parser);

    if (!match_token(parser, TOKEN_SWITCH)) {
        parser_error(parser, "Expected 'switch'.");
    }

    if (!match_token(parser, TOKEN_BEGINPARAMS)) {
        parser_error(parser, "Expected '(' after 'switch'.");
    }

    // Create the switch node
    ASTNode* switchNode = create_ast_node(AST_SWITCH, switchTok.line, switchTok.column, NULL);

    // Parse the switch condition
    ASTNode* condition = parse_expression(parser);
    if (!condition) {
        parser_error(parser, "Expected expression in switch statement.");
    }
    ast_add_child(switchNode, condition);

    if (!match_token(parser, TOKEN_ENDPARAMS)) {
        parser_error(parser, "Expected ')' after switch condition.");
    }

    if (!match_token(parser, TOKEN_BEGINBLOCK)) {
        parser_error(parser, "Expected '{' to begin switch block.");
    }

    int hasCases = 0;      // Track if at least one 'when' case is parsed.
    int hasDefault = 0;    // Track if the 'default' case is parsed.

    while (1) {
        Token t = peek_token(parser);
        if (t.type == TOKEN_ENDBLOCK) {
            consume_token(parser); // consume '}'
            break;
        }
        if (t.type == TOKEN_EOF) {
            parser_error(parser, "Unexpected EOF in switch block; missing '}'.");
        }

        // Parse 'when' case
        if (match_token(parser, TOKEN_WHEN)) {
            hasCases = 1; // Mark that we have at least one case.
            Token whenTok = t;
            ASTNode* whenNode = create_ast_node(AST_WHEN, whenTok.line, whenTok.column, NULL);

            // Parse the case value
            ASTNode* caseValue = parse_expression(parser);
            if (!caseValue) {
                parser_error(parser, "Expected value after 'when'.");
            }
            ast_add_child(whenNode, caseValue);

            if (!match_token(parser, TOKEN_COLON)) {
                parser_error(parser, "Expected ':' after 'when' value.");
                synchronize_to_next_case(parser);
                continue;
            }

            // Parse the statements inside the 'when' case
            parse_case_statements(parser, whenNode);
            ast_add_child(switchNode, whenNode);
        }
        // Parse 'default' case
        else if (match_token(parser, TOKEN_DEFAULT)) {
            if (hasDefault) {
                parser_error(parser, "Duplicate 'default' case in switch block.");
            }
            hasDefault = 1;

            Token defaultTok = t;
            ASTNode* defaultNode = create_ast_node(AST_DEFAULT, defaultTok.line, defaultTok.column, NULL);

            if (!match_token(parser, TOKEN_COLON)) {
                parser_error(parser, "Expected ':' after 'default'.");
            }

            // Parse the statements inside the 'default' case
            parse_case_statements(parser, defaultNode);
            ast_add_child(switchNode, defaultNode);
        }
        // Synchronize on error
        else {
            parser_error(parser, "Expected 'when' or 'default' in switch block.");
            synchronize_to_next_case(parser);
        }
    }

    if (!hasCases && !hasDefault) {
        parser_error(parser, "Switch block must contain at least one 'when' or 'default' case.");
    }

    return switchNode;
}




/***********************************************************
* Function: parse_array_index
* Description: this function parses the INDEX of an array
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_array_index(Parser* parser) {
    Token t = peek_token(parser);

    if (t.type != TOKEN_BEGININDEX) { // Ensure the token is '['
        parser_error(parser, "Error: Expected '[' for array indexing.");
    }

    consume_token(parser); // Consume '['

    // Peek the next token to determine the type of the index
    Token indexToken = peek_token(parser);

    ASTNode* indexNode = NULL;

    if (indexToken.type == TOKEN_NUMBER) { // Integer literal
        consume_token(parser); // Consume the number token
        indexNode = create_ast_node(
            AST_LITERAL, indexToken.line, indexToken.column, NULL);
        long indexValue = strtol(indexToken.value, NULL, 10); // Convert string to integer
        ast_node_set_int(indexNode, indexValue);
    }
    else if (indexToken.type == TOKEN_IDENTIFIER) { // Variable/identifier
        consume_token(parser); // Consume the identifier token
        indexNode = create_ast_node(
            AST_IDENTIFIER, indexToken.line, indexToken.column, indexToken.value);
    }
    else {
        parser_error(parser, "Error: Array index must be an integer or identifier.");
    }

    // Expect the closing ']'
    if (!match_token(parser, TOKEN_ENDINDEX)) { // Ensure token is ']'
        parser_error(parser, "Error: Unterminated array access (missing ']').");
    }

    return indexNode;
}




/***********************************************************
* Function: parse_for_statement
* Description: this function parses the for statement
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_for_statement(Parser* parser) {
    Token fTok = peek_token(parser);
    if (!match_token(parser, TOKEN_FOR)) {
        parser_error(parser, "Expected 'for'.");
    }

    if (!match_token(parser, TOKEN_BEGINPARAMS)) {
        parser_error(parser, "Expected '(' after 'for'.");
    }

    ASTNode* forNode = create_ast_node(AST_FOR_STATEMENT,
        fTok.line, fTok.column,
        NULL);

    /* Instead of parse_binary, we use parse_expression for the start and end. */
    ASTNode* startExpr = parse_expression(parser);
    if (!startExpr) {
        parser_error(parser, "Invalid start expression in 'for'.");
    }
    ast_add_child(forNode, startExpr);

    /* expect 'to' */
    if (!match_token(parser, TOKEN_TO)) {
        parser_error(parser, "Expected 'to' in 'for'.");
    }

    ASTNode* endExpr = parse_expression(parser);
    if (!endExpr) {
        parser_error(parser, "Invalid end expression in 'for'.");
    }
    ast_add_child(forNode, endExpr);

    /* expect ')' */
    if (!match_token(parser, TOKEN_ENDPARAMS)) {
        parser_error(parser, "Expected ')' after 'for(...)'.");
    }

    /* parse block */
    ASTNode* blockStmt = parse_block(parser);
    if (!blockStmt) {
        parser_error(parser, "Missing block after 'for(...)'.");
    }
    ast_add_child(forNode, blockStmt);

    return forNode;
}






/***********************************************************
* Function: parse_function_declaration
* Description: this function parses the function declaration
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_function_declaration(Parser* parser) {
    Token funcTok = peek_token(parser);
    if (!match_token(parser, TOKEN_FUNCTION)) {
        parser_error(parser, "Expected 'function'.");
    }

    Token nameTok = consume_token(parser);
    if (nameTok.type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected function name after 'function'.");
    }

    if (!match_token(parser, TOKEN_BEGINPARAMS)) {
        parser_error(parser, "Expected '(' after function name.");
    }

    /* Create the function node. */
    ASTNode* funcNode = create_ast_node(AST_FUNCTION_DECLARATION,
        funcTok.line,
        funcTok.column,
        NULL);

    /* Create the identifier node and add it as a child to the function node */
    ASTNode* identifierNode = create_ast_node(AST_IDENTIFIER,
        nameTok.line,
        nameTok.column,
        nameTok.value);
    ast_add_child(funcNode, identifierNode);

    /* parse parameters */
    parse_function_params(parser, funcNode);

    if (!match_token(parser, TOKEN_ENDPARAMS)) {
        parser_error(parser, "Expected ')' after function parameters.");
    }

    /* parse body as block */
    ASTNode* body = parse_block(parser);
    if (!body) {
        parser_error(parser, "Missing function body '{ ... }'.");
    }
    ast_add_child(funcNode, body);

    return funcNode;
}






/***********************************************************
* Function: parse_function_params
* Description: this function parses the function parameters
* Parameters: Parser* parser, ASTNode* funcNode
* Return: void
* ***********************************************************/
void parse_function_params(Parser* parser, ASTNode* funcNode) {
    while (1) {
        Token t = peek_token(parser);
        if (t.type == TOKEN_ENDPARAMS || t.type == TOKEN_EOF) {
            /* no more params */
            break;
        }
        /* must be an identifier */
        Token paramTok = consume_token(parser);
        if (paramTok.type != TOKEN_IDENTIFIER) {
            parser_error(parser, "Expected identifier in function parameter list.");
        }

        ASTNode* paramNode = create_ast_node(AST_IDENTIFIER,
            paramTok.line, paramTok.column,
            paramTok.value);
        ast_add_child(funcNode, paramNode);

        /* if next is ')' => done, else expect a comma or 'end' */
        Token nextT = peek_token(parser);
        if (nextT.type == TOKEN_ENDPARAMS) {
            break;
        }
        else {
            /* in your grammar, a comma might be TOKEN_END or some custom token */
            if (!match_token(parser, TOKEN_SEPARATION)) {
                parser_error(parser, "Expected ',' between parameters.");
            }
        }
    }
}




/***********************************************************
* Function: parse_expression
* Description: this function parses the expressions
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_expression(Parser* parser) {
    Token t = peek_token(parser);
    if (t.type == TOKEN_EOF) {
        parser_error(parser, "Unexpected EOF in expression.");
    }
    return parse_binary(parser, 0);
}






/***********************************************************
* Function: parse_binary
* Description: this function parses the binary expressions
* Parameters: Parser* parser, int precedence
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_binary(Parser* parser, int precedence) {
    ASTNode* left = parse_unary(parser);
    if (!left) return NULL;

    while (1) {
        Token t = peek_token(parser);
        /* If next token doesn't form a binary expression or has lower precedence, break */
        if (t.type == TOKEN_END || t.type == TOKEN_ENDBLOCK ||
            t.type == TOKEN_EOF || t.type == TOKEN_TO ||
            t.type == TOKEN_BEGINPARAMS || t.type == TOKEN_ENDPARAMS || t.type == TOKEN_COLON)
        {
            break;
        }

        int prec = get_precedence(t.type);
        if (prec < precedence) {
            break;
        }

        /* consume operator */
        consume_token(parser);

        /* parse right operand with higher precedence */
        ASTNode* right = parse_binary(parser, prec + 1);
        if (!right) {
            free_ast_node(left);
            return NULL;
        }

        /* build binary node */
        ASTNode* bin = create_ast_node(AST_BINARY_EXPR,
            t.line, t.column,
            t.value);
        ast_add_child(bin, left);
        ast_add_child(bin, right);
        left = bin;
    }

    return left;
}





/***********************************************************
* Function: parse_unary
* Description: this function parses the unary expressions
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_unary(Parser* parser) {
    Token t = peek_token(parser);

    if (t.type == TOKEN_MINUS ||
        t.type == TOKEN_COMPLEMENT ||
        t.type == TOKEN_AND ||
        t.type == TOKEN_MULTIPLY)
    {
        consume_token(parser);
        ASTNode* operand = parse_unary(parser);
        if (!operand) {
            parser_error(parser, "Invalid operand for unary operator.");
        }
        ASTNode* un = create_ast_node(AST_UNARY_EXPR,
            t.line, t.column,
            t.value);
        ast_add_child(un, operand);
        return un;
    }
    return parse_postfix(parser);
}



/***********************************************************
* Function: parse_postfix
* Description: this function parses the postfix expressions
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_postfix(Parser* parser) {
    ASTNode* node = parse_primary(parser); // Parse the primary expression first
    if (!node) return NULL;

    while (1) {
        Token t = peek_token(parser);

        /* Handle function calls: ident(...) */
        if (t.type == TOKEN_BEGINPARAMS) { // '('
            consume_token(parser); // Consume '('

            /* Create a new AST_FUNCTION_CALL node */
            ASTNode* callNode = create_ast_node(
                AST_FUNCTION_CALL, t.line, t.column, NULL);

            /* Add the current node (identifier or function) as the first child */
            ast_add_child(callNode, node);

            /* Parse arguments until ')' */
            while (!match_token(parser, TOKEN_ENDPARAMS)) { // ')'
                Token argPeek = peek_token(parser);
                if (argPeek.type == TOKEN_EOF) {
                    parser_error(parser, "Error: Unterminated function call (missing ')').");
                }

                /* Parse the argument expression */
                ASTNode* arg = parse_expression(parser);
                if (!arg) {
                    parser_error(parser, "Error: Invalid expression in function call.");
                }
                ast_add_child(callNode, arg);

                /* Check for ',' or ')' */
                if (peek_token(parser).type == TOKEN_ENDPARAMS) {
                    // Closing parenthesis detected, no further action needed
                }
                else if (!match_token(parser, TOKEN_END)) { // ','
                    parser_error(parser, "Error: Expected ',' between arguments.");
                }
            }

            /* Replace the current node with the function call node */
            node = callNode;
        }

        /* Handle pointer member access: expr->member */
        else if (t.type == TOKEN_POINTER) { // '->'
            consume_token(parser); // Consume '->'

            Token member = peek_token(parser);
            if (member.type != TOKEN_IDENTIFIER) {
                free_ast_node(node); // Free the previously allocated node
                parser_error(parser, "Error: Expected identifier after '->'.");
            }
            consume_token(parser); // Consume the member identifier

            /* Create an AST_BINARY_EXPR node for '->' */
            ASTNode* ptrAccess = create_ast_node(
                AST_BINARY_EXPR, t.line, t.column, "->");

            /* Add the current node (left-hand side) as the first child */
            ast_add_child(ptrAccess, node);

            /* Create a new AST_IDENTIFIER node for the member and add it as the second child */
            ASTNode* memNode = create_ast_node(
                AST_IDENTIFIER, member.line, member.column, member.value);
            ast_add_child(ptrAccess, memNode);

            /* Replace the current node with the pointer access node */
            node = ptrAccess;
        }

        /* Handle array indexing: expr[index] */
        else if (t.type == TOKEN_BEGININDEX) { // '['
            consume_token(parser); // Consume '['

            /* Parse the index, which can be an integer literal or identifier */
            Token indexToken = peek_token(parser);
            ASTNode* indexNode = NULL;

            if (indexToken.type == TOKEN_NUMBER) { // Integer literal
                consume_token(parser); // Consume the number token
                indexNode = create_ast_node(
                    AST_LITERAL, indexToken.line, indexToken.column, NULL);
                long indexValue = strtol(indexToken.value, NULL, 10); // Convert string to integer
                ast_node_set_int(indexNode, indexValue);
            }
            else if (indexToken.type == TOKEN_IDENTIFIER) { // Identifier
                consume_token(parser); // Consume the identifier token
                indexNode = create_ast_node(
                    AST_IDENTIFIER, indexToken.line, indexToken.column, indexToken.value);
            }
            else {
                parser_error(parser, "Error: Array index must be an integer or identifier.");
            }

            /* Expect the closing ']' */
            if (!match_token(parser, TOKEN_ENDINDEX)) { // ']'
                parser_error(parser, "Error: Unterminated array access (missing ']').");
            }

            /* Create an AST_ARRAY_ACCESS node */
            ASTNode* arrayAccess = create_ast_node(
                AST_ARRAY_ACCESS, t.line, t.column, NULL);

            /* Add the array and index as children */
            ast_add_child(arrayAccess, node);      // The array is the left-hand side
            ast_add_child(arrayAccess, indexNode); // The index is the right-hand side

            /* Replace the current node with the array access node */
            node = arrayAccess;
        }

        /* No matching postfix operator, break the loop */
        else {
            break;
        }
    }

    return node;
}





/***********************************************************
* Function: parse_primary
* Description: this function parses the primary expressions
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_primary(Parser* parser) {
    Token t = peek_token(parser);
    ASTNode* lit = NULL;
    if (t.type == TOKEN_EOF) {
        return NULL;
    }

    switch (t.type) {
        // Handle literals
    case TOKEN_BINARY:
        consume_token(parser);
        lit = create_ast_node(AST_LITERAL, t.line, t.column, NULL);
        ast_node_set_int(lit, parse_binary_string(t.value));
        return lit;

    case TOKEN_HEX:
    case TOKEN_NUMBER: {
        consume_token(parser);
        lit = create_ast_node(AST_LITERAL, t.line, t.column, NULL);
        long val = strtol(t.value, NULL, 0);
        ast_node_set_int(lit, val);
        return lit;
    }

    case TOKEN_FLOAT: {
        consume_token(parser);
        lit = create_ast_node(AST_LITERAL, t.line, t.column, NULL);
        double val = strtod(t.value, NULL);
        ast_node_set_float(lit, val);
        return lit;
    }

    case TOKEN_CHAR: {
        consume_token(parser);
        lit = create_ast_node(AST_LITERAL, t.line, t.column, NULL);
        if (t.value && t.value[0]) {
            long c = (unsigned char)t.value[0];
            ast_node_set_int(lit, c);
        }
        return lit;
    }

    case TOKEN_STRING: {
        consume_token(parser);
        lit = create_ast_node(AST_LITERAL, t.line, t.column, NULL);
        ast_node_set_string(lit, t.value);
        return lit;
    }

    case TOKEN_TRUE:
    case TOKEN_FALSE: {
        consume_token(parser);
        lit = create_ast_node(AST_LITERAL, t.line, t.column, NULL);
        ast_node_set_bool(lit, (t.type == TOKEN_TRUE));
        return lit;
    }

                    // Handle identifiers (including potential array accesses)
    case TOKEN_IDENTIFIER: {
        consume_token(parser);
        ASTNode* ident = create_ast_node(AST_IDENTIFIER, t.line, t.column, t.value);
        return ident;
    }

    case TOKEN_BREAK: {
        consume_token(parser);
        ASTNode* breakNode = create_ast_node(AST_BREAK, t.line, t.column, NULL);
        return breakNode;
    }

    case TOKEN_CONTINUE: {
        consume_token(parser);
        ASTNode* continueNode = create_ast_node(AST_CONTINUE, t.line, t.column, NULL);
        return continueNode;
    }

    default:
        parser_error(parser, "Unexpected token in primary expression.");
        return NULL;
    }
}




/***********************************************************
* Function: parse_function_call
* Description: this function parses the function call
* Parameters: Parser* parser, ASTNode* identifier
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_array_access(Parser* parser, ASTNode* identifier) {
    // Loop to handle multiple chained accesses like myArray[2][3]
    while (peek_token(parser).type == TOKEN_BEGININDEX) { // '[' token
        Token startTok = consume_token(parser); // Consume '['

        // Parse the index expression inside the brackets
        ASTNode* indexExpr = parse_expression(parser);
        if (!indexExpr) {
            parser_error(parser, "Expected expression inside array access brackets.");
        }

        // Expect closing bracket ']'
        if (!match_token(parser, TOKEN_ENDINDEX)) { // ']' token
            parser_error(parser, "Expected ']' to close array access.");
        }

        // Create a new AST node for the array access
        ASTNode* accessNode = create_ast_node(AST_ARRAY_ACCESS, startTok.line, startTok.column, NULL);

        // Add the array identifier (or previous access) as the first child
        ast_add_child(accessNode, identifier);

        // Add the index expression as the second child
        ast_add_child(accessNode, indexExpr);

        // Replace identifier with the new access node for chaining
        identifier = accessNode;
    }

    return identifier;
}





/***********************************************************
* Function: get_precedence
* Description: this function gets the precedence of the token
* Parameters: TokenKind type
* Return: int
* ***********************************************************/
int get_precedence(TokenKind type) {
    switch (type) {
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
        return 4;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_MOD:
        return 3;
    case TOKEN_LESSTHAN:
    case TOKEN_LESSTHANOREQUALS:
    case TOKEN_GREATERTHAN:
    case TOKEN_GREATERTHANOREQUALS:
        return 2;
    case TOKEN_EQUALS_EQUALS:
    case TOKEN_NOT_EQUALS:
        return 1;
    default:
        return 0;
    }
}




/***********************************************************
* Function: parse_array_list
* Description: this function parses the array list
* Parameters: Parser* parser
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_array_list(Parser* parser) {
    Token startTok = peek_token(parser);

    if (!match_token(parser, TOKEN_BEGINBLOCK)) { // Expect '{'
        parser_error(parser, "Expected '{' to begin array initialization.");
    }

    ASTNode* arrayNode = create_ast_node(AST_ARRAY_LITERAL, startTok.line, startTok.column, NULL);

    while (1) {
        Token t = peek_token(parser);
        if (t.type == TOKEN_ENDBLOCK) { // Expect '}'
            consume_token(parser); // Consume '}'
            break;
        }

        if (t.type == TOKEN_EOF) {
            parser_error(parser, "Unexpected EOF in array initialization.");
        }

        // Parse each element as an expression
        ASTNode* element = parse_expression(parser);
        if (!element) {
            parser_error(parser, "Invalid expression in array initialization.");
        }

        ast_add_child(arrayNode, element);

        // Expect ',' or '}'
        t = peek_token(parser);
        if (t.type == TOKEN_END) {
            consume_token(parser); // Consume ',' and continue to the next element
        }
        else if (t.type != TOKEN_ENDBLOCK) {
            parser_error(parser, "Expected ',' or '}' in array initialization.");
        }
    }

    return arrayNode;
}




/***********************************************************
* Function: parse_var_declaration
* Description: this function parses the variable declaration
* Parameters: Parser* parser, int is_array
* Return: ASTNode*
* ***********************************************************/
ASTNode* parse_var_declaration(Parser* parser, int is_array) {
    Token startTok = peek_token(parser);

    if (is_array) {
        if (!match_token(parser, TOKEN_ARRAY)) {
            parser_error(parser, "Expected 'list'.");
        }


    }
    else {
        if (!match_token(parser, TOKEN_VAR)) {
            parser_error(parser, "Expected 'make'."); // Or "var"
        }
    }

    Token varName = consume_token(parser);
    if (varName.type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected identifier after 'make' or 'list'.");
    }

    if (!match_token(parser, TOKEN_EQUALS)) {
        parser_error(parser, "Expected '=' in variable declaration.");
    }

    ASTNode* init = is_array ? parse_array_list(parser) : parse_expression(parser);
    if (!init) {
        parser_error(parser, "Expected initializer expression.");
    }

    if (!match_token(parser, TOKEN_END)) {
        free_ast_node(init);
        parser_error(parser, "Expected ';' after variable declaration.");
    }

    /* Represent a var declaration as an assignment node with operator '=' */
    ASTNode* decl = create_ast_node(AST_ASSIGNMENT,
        startTok.line, startTok.column,
        "=");

    ASTNode* identNode = create_ast_node(AST_IDENTIFIER,
        varName.line,
        varName.column,
        varName.value);
    ast_add_child(decl, identNode);
    ast_add_child(decl, init);

    return decl;
}




/***********************************************************
* Function: parse_binary_string
* Description: this function parses the binary string
* Parameters: const char* binStr
* Return: long
* ***********************************************************/
long parse_binary_string(const char* binStr) {
    long result = 0;
    for (size_t i = 2; binStr[i] != '\0'; i++) {
        char c = binStr[i];
        result <<= 1;  // multiply current result by 2
        if (c == '1') {
            result += 1;
        }
        else if (c != '0') {
            // Not a valid binary digit
            // we could throw an error or handle gracefully
            fprintf(stderr, "Invalid binary digit '%c'\n", c);
            // we’ll just ignore or break
        }
    }
    return result;
}