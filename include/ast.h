#pragma once
/***********************************************************
* File: ast.h
* This file contains the Abstract Syntax Tree (AST) for the interpreter.
* The AST is a tree representation of the source code.
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/

#pragma once

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>


/**
 * The type of each AST node.
 */
typedef enum {
    AST_PROGRAM,              // Root of the program
    AST_BLOCK,                // Block of statements
    AST_BINARY_EXPR,          // Binary expression (e.g., a + b)
    AST_UNARY_EXPR,           // Unary expression (e.g., -a, !b)
    AST_LITERAL,              // Literal (numbers, strings, booleans)
    AST_IDENTIFIER,           // Identifier (e.g., variable name)
    AST_ASSIGNMENT,           // Assignment statement (e.g., a = b)
    AST_IF_STATEMENT,         // If-else statement
    AST_WHILE_STATEMENT,      // While loop
    AST_FOR_STATEMENT,        // For loop
    AST_RETURN_STATEMENT,     // Return statement
    AST_FUNCTION_DECLARATION, // Function declaration
    AST_FUNCTION_CALL,        // Function call
    AST_ARRAY_LITERAL,        // Array literal
    AST_ARRAY_ACCESS,         // Array access
    AST_COMMENT,			  // Comment
    AST_BREAK,				  // Break statement
    AST_CONTINUE,			  // Continue statement
    AST_SWITCH,				  // Switch statement
    AST_WHEN,				  // When statement
    AST_PARAMETER_LIST,       // Parameter list for function declarations
    AST_DEFAULT,              // Default case for switch statement
} ASTNodeType;





/**
 * A tag for which type of data is stored in the ASTValue union
 * (used only if this node is AST_LITERAL or otherwise stores a value).
 */
typedef enum {
    VALUE_NONE,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_BOOL,
    VALUE_STRING
} ASTValueKind;

/**
 * A union that can hold different kinds of literal data.
 */
typedef union {
    long   int_val;    // For integer literals
    double float_val;  // For floating-point literals
    bool   bool_val;   // For boolean literals
    char* str_val;    // For string literals
} ASTValue;

/**
 * The primary AST node structure.
 */
typedef struct ASTNode {
    ASTNodeType   type;       // The type of this node (e.g., AST_LITERAL, AST_BINARY_EXPR)

    ASTValueKind  value_kind; // If this is a literal, indicates which member of 'value' is used
    ASTValue      value;      // The literal data (if any)

    char* operator_;  // Operator symbol (e.g. "+", "-", "==") for expression nodes
    // Children: We store all children in a dynamic array, which can include
    // the left and right sides of a binary expression or multiple statements in a block.
    struct ASTNode** children;
    size_t        child_count;
	bool isFunction;

    // So we can easily find the parent node when needed.
    struct ASTNode* parent;

    // Source code information for error reporting or debugging.
    size_t line;
    size_t column;
} ASTNode;

typedef struct {
    const ASTNode* node;   // Original AST node
    int parent_index;      // Index of the parent in the flat list (-1 for root)
    int* children_indices; // Dynamic array of child indices
    size_t child_count;    // Number of children
} FlatNode;



/**
 * Creates a new AST node with the given type, line, and column.
 * Optionally specify an operator (e.g., "+", "-") if relevant.
 */
ASTNode* create_ast_node(ASTNodeType type, size_t line, size_t column, const char* operator_);

/**
 * These functions set the literal value in the node and update value_kind.
 */
void ast_node_set_int(ASTNode* node, long value);
void ast_node_set_float(ASTNode* node, double value);
void ast_node_set_bool(ASTNode* node, bool value);
void ast_node_set_string(ASTNode* node, const char* str);

/**
 * Adds a child node to the parent's children array.
 * Automatically manages the array size and sets child's parent pointer.
 */
void ast_add_child(ASTNode* parents, ASTNode* child);

/**
 * Frees an AST node and all its descendants (children, grandchildren, etc.).
 */
void free_ast_node(ASTNode* node);

/**
 * Prints the AST (for debugging). 'depth' indicates indentation level.
 */
void print_ast(const ASTNode* node, int depth);

char* str_duplicate(const char* src);
void print_flattened_ast(FlatNode* flat_list, size_t flat_count);
void flatten_ast(const ASTNode* root, FlatNode** flat_list, size_t* count, int parent_index);


#endif // AST_H
