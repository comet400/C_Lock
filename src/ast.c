/***********************************************************
* File: ast.c
* This file contains the Abstract Syntax Tree (AST) for the interpreter.
* The AST is a tree representation of the source code.
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/

#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/**
 * The type of each AST node.
 */
const char* ASTNodeTypeNames[] = {
    "AST_PROGRAM",
    "AST_BLOCK",
    "AST_BINARY_EXPR",
    "AST_UNARY_EXPR",
    "AST_LITERAL",
    "AST_IDENTIFIER",
    "AST_ASSIGNMENT",
    "AST_IF_STATEMENT",
    "AST_WHILE_STATEMENT",
    "AST_FOR_STATEMENT",
    "AST_RETURN_STATEMENT",
    "AST_FUNCTION_DECLARATION",
    "AST_FUNCTION_CALL",
    "AST_ARRAY_LITERAL",
    "AST_ARRAY_ACCESS",
    "AST_COMMENT",
    "AST_BREAK",
    "AST_CONTINUE",
    "AST_SWITCH",
    "AST_WHEN",
    "AST_PARAMETER_LIST",
    "AST_DEFAULT",
};


/***********************
 * AST Node Functions
 ***********************/

 /***********************************************************
 * Function: str_duplicate
 * Description: this function duplicates a string. refactored from lexer.c, we can use strndump or str_duplicate
 * Parameters: const char* str
 * Return: const char*
 * ***********************************************************/
char* str_duplicate(const char* src) {
    if (!src) return NULL;
    size_t len = strlen(src);
    char* dup = (char*)malloc(len + 1);
    if (!dup) {
        fprintf(stderr, "Memory allocation failed in str_duplicate\n");
        exit(EXIT_FAILURE);
    }

        #ifdef _WIN32
            strcpy_s(dup, len + 1, src);
        #else
            strncpy(dup, src, len);
            dup[len] = '\0';
        #endif

    return dup;
}





/***********************************************************
 * Function: create_ast_node
 * Description: this function creates a new AST node.
 * Parameters: ASTNodeType type, size_t line, size_t column, const char* operator_
 * Return: ASTNode*
 * ***********************************************************/
ASTNode* create_ast_node(ASTNodeType type, size_t line, size_t column, const char* operator_) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_ast_node\n");
        exit(EXIT_FAILURE);
    }

    node->type = type;
    node->value_kind = VALUE_NONE; // Default: no literal value
    // Initialize the union to something safe:
    node->value.int_val = 0;

    node->operator_ = str_duplicate(operator_);

    node->children = NULL;
    node->child_count = 0;
    node->parent = NULL;
    node->line = line;
    node->column = column;

    return node;
}






/***********************************************************
 * Function: ast_node_set_int
 * Description: this function sets the literal value in the node and update value_kind.
 * Parameters: ASTNode* node, long value
 * Return: void
 * ***********************************************************/
void ast_node_set_int(ASTNode* node, long value) {
    node->value_kind = VALUE_INT;
    node->value.int_val = value;
}





/***********************************************************
 * Function: ast_node_set_float
 * Description: this function sets the literal value in the node and update value_kind.
 * Parameters: ASTNode* node, double value
 * Return: void
 * ***********************************************************/
void ast_node_set_float(ASTNode* node, double value) {
    node->value_kind = VALUE_FLOAT;
    node->value.float_val = value;
}





/***********************************************************
 * Function: ast_node_set_bool
 * Description: this function sets the literal value in the node and update value_kind.
 * Parameters: ASTNode* node, bool value
 * Return: void
 * ***********************************************************/
void ast_node_set_bool(ASTNode* node, bool value) {
    node->value_kind = VALUE_BOOL;
    node->value.bool_val = value;
}





/***********************************************************
 * Function: ast_node_set_string
 * Description: this function sets the literal value in the node and update value_kind.
 * Parameters: ASTNode* node, const char* str
 * Return: void
 * ***********************************************************/
void ast_node_set_string(ASTNode* node, const char* str) {
    node->value_kind = VALUE_STRING;
    node->value.str_val = str_duplicate(str);
}






/***********************************************************
 * Function: ast_add_child
 * Description: this function adds a child node to the parent's children array.
 * Parameters: ASTNode* parentt, ASTNode* child
 * Return: void
 * ***********************************************************/
void ast_add_child(ASTNode* parentt, ASTNode* child) {
    if (!parentt || !child) return;

    parentt->children = (ASTNode**)realloc(parentt->children,
        sizeof(ASTNode*) * (parentt->child_count + 1));
    if (!parentt->children) {
        fprintf(stderr, "Memory allocation failed in ast_add_child\n");
        exit(EXIT_FAILURE);
    }
    parentt->children[parentt->child_count] = child;
    parentt->child_count++;

    // Set the child's parent pointer (if you want upward navigation).
    child->parent = parentt;
}






/***********************************************************
 * Function: free_ast_node
 * Description: this function frees the memory used by the AST node.
 * Parameters: ASTNode* node
 * Return: void
 * ***********************************************************/
void free_ast_node(ASTNode* node) {
    if (!node) return;

    // If the node is a string literal, free the stored string
    if (node->value_kind == VALUE_STRING) {
        free(node->value.str_val);
        node->value.str_val = NULL;
    }

    // Free operator string
    free(node->operator_);
    node->operator_ = NULL;

    // Recursively free children
    for (size_t i = 0; i < node->child_count; i++) {
        free_ast_node(node->children[i]);
    }
    free(node->children);
    node->children = NULL;

    // Finally, free this node
    free(node);
}






/***********************************************************
 * Function: print_ast
 * Description: this function prints the AST node.
 * Parameters: const ASTNode* node, int depth
 * Return: void
 * ***********************************************************/
void print_ast(const ASTNode* node, int depth) {
    if (!node) return;

    // Indentation based on depth
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    // Print node type, line/column
    printf("Node Type: %s, Value Kind: %d, Line: %zu, Column: %zu",
        ASTNodeTypeNames[node->type], node->value_kind, node->line, node->column);

    // Print operator if exists
    if (node->operator_) {
        printf(", Operator: '%s'", node->operator_);
    }

    // Print any literal value
    switch (node->value_kind) {
    case VALUE_INT:
        printf(", Int Value: %ld", node->value.int_val);
        break;
    case VALUE_FLOAT:
        printf(", Float Value: %f", node->value.float_val);
        break;
    case VALUE_BOOL:
        printf(", Bool Value: %s", node->value.bool_val ? "true" : "false");
        break;
    case VALUE_STRING:
        printf(", String Value: \"%s\"", node->value.str_val);
        break;
    case VALUE_NONE:
    default:
        // no literal
        break;
    }

    // End the line for this node
    printf("\n");

    // Print children recursively
    for (size_t i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], depth + 1);
    }
}
