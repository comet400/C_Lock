#ifndef BYTECODE_H
#define BYTECODE_H

#include <stddef.h>
#include <stdint.h>

// Define opcodes for bytecode instructions
typedef enum {
    OP_HALT,       // Halt execution
    OP_LOAD_INT,   // Load an integer onto the stack
    OP_ADD,        // Add two values on the stack
    OP_SUB,        // Subtract two values on the stack
    OP_MUL,        // Multiply two values on the stack
    OP_DIV,        // Divide two values on the stack
    OP_PRINT,      // Print the top value on the stack
    OP_JUMP,       // Unconditional jump
    OP_JUMP_IF_ZERO, // Conditional jump if top of stack is zero
    OP_STORE_VAR,  // Store stack top into a variable
    OP_LOAD_VAR    // Load a variable onto the stack
} OpCode;

// Bytecode instruction structure
typedef struct {
    OpCode op;    // The operation code
    int operand;  // Optional operand (e.g., constant value or jump address)
} Instruction;

// Bytecode structure to hold a program
typedef struct {
    Instruction* instructions; // Array of instructions
    size_t count;              // Number of instructions
    size_t capacity;           // Allocated capacity for instructions
} Bytecode;

// Function prototypes
Bytecode* create_bytecode(size_t initial_capacity);
void append_instruction(Bytecode* bytecode, OpCode op, int operand);
void free_bytecode(Bytecode* bytecode);
void print_bytecode(const Bytecode* bytecode);

#endif // BYTECODE_H
