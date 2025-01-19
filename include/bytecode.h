/***********************************************************
* File: bytecode.h
* This file have the bytecode for the interpreter.
* The bytecode is used to store the instructions for the machine compiler (machine type: 86x64).
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/




#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

#ifndef BYTECODE_H
#define BYTECODE_H

//just so i keep track of it
#define OP_ADD        0x04   // Add two registers
#define OP_SUB        0x07   // Subtract two registers
#define OP_MUL        0x08   // Multiply two registers
#define OP_DIV        0x09   // Divide two registers
#define OP_MOD        0x0A   // Modulo
#define OP_NEG        0x0B   // Negate a register value


#define OP_LOAD_CONST 0x01   // Load a constant into a register
#define OP_LOAD_VAR   0x02   // Load a variable into a register
#define OP_STORE_VAR  0x03   // Store a register value into a variable

#define OP_ARRAY_GET  0x0C   // Load an array element
#define OP_ARRAY_SET  0x0D   // Store a value into an array element

#define OP_JUMP          0x05   // Unconditional jump to a specific instruction
#define OP_JUMP_IF_TRUE  0x0E   // Jump if the top of the stack is true
#define OP_JUMP_IF_FALSE 0x0F   // Jump if the top of the stack is false

#define OP_CALL    0x10   // Call a function
#define OP_RETURN  0x06   // Return from a function

#define OP_AND     0x11   // Logical AND
#define OP_OR      0x12   // Logical OR
#define OP_NOT     0x13   // Logical NOT

#define OP_EQ      0x14   // Equal to
#define OP_NEQ     0x15   // Not equal to
#define OP_LT      0x16   // Less than
#define OP_GT      0x17   // Greater than
#define OP_LTE     0x18   // Less than or equal to
#define OP_GTE     0x19   // Greater than or equal to

#define OP_PRINT   0x1A   // Print a value
#define OP_INPUT   0x1B   // Read a value from the user

#define INITIAL_BYTECODE_CAPACITY 1024
typedef enum {
    OP_PUSH_INT,
    OP_PUSH_FLOAT,
    OP_PUSH_BOOL,
    OP_PUSH_STRING,
    OP_ADD_,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_ASSIGN,
    OP_JUMP_,
    OP_JUMP_TO_IF_FALSE,
    OP_CALL_FUNCTION,
	OP_DECL_FUNCTION,
    OP_RETURN_,
    OP_NEGATE,
    OP_MODULO,
	OP_MODULO_EQUAL,
    OP_LESS,
    OP_GREATER,
    OP_LESS_EQUAL,
    OP_GREATER_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_AND_,
    OP_OR_,
    OP_NOT_,
    OP_PRINT_,
    OP_INPUT_,
    OP_ARRAY_SET_,
    OP_ARRAY_GET_,
    OP_LOAD_VAR_,
    OP_STORE_VAR_,
    OP_LOAD_CONST_,
    OP_POP,
    OP_HALT,
    OP_JUMP_IF_TRUE_,
    OP_JUMP_IF_FALSE_,
    OP_JUMP_TO,
    OP_CALL_,
    OP_RETURN_STM,
    OP_BREAK_,
    OP_CONTINUE_,
    OP_SWITCH_,
    OP_WHEN_,
    OP_DEFAULT_,
    OP_PARAMETER_LIST_,
	OP_SCOPE
} BytecodeOpcode;

typedef struct {
    BytecodeOpcode opcode;
    union {
        int int_operand;
        float float_operand;
        bool bool_operand;
        char* string_operand;
        // For jumps and function calls
        struct {
            int target_index;
            int arg_count;
        } jump;
        // For binary operations
        struct {
            int left_reg;
            int right_reg;
        } binary;
        // For unary operations
        struct {
            int reg;
        } unary;
        // For array access
        struct {
            int array_reg;
            int index_reg;
        } array_access;
        // For array literals
        struct {
            int array_reg;
            int count;
        } array_literal;
        // For function declarations
        struct {
            int param_count;
            int body_index;
        } function_decl;
        // For switch statements
        struct {
            int default_index;
            int when_count;
        } switch_;
        // For when statements
        struct {
            int condition_reg;
            int body_index;
        } when;
        // For parameter lists
        struct {
            int param_count;
        } param_list;

        struct {
            int line_number;
            int column_number;
        } debug_info;

        struct {
            int local_index;
            int global_index;
        } scope;

    } operand;

} BytecodeInstruction;


void generate_literal_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_binary_expr_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_assignment_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void ensure_bytecode_capacity(BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void print_byteCode(const BytecodeInstruction* bytecode, size_t count);
void generate_identifier_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_unary_exp_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_if_statement_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity); 
void generate_for_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_while_loop_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_function_declaration_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_function_call_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);
void generate_return_statement_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity);


#endif BYTECODE_H