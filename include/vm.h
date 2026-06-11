#pragma once
#ifndef VM_H
#define VM_H

#include <stddef.h>
#include <stdbool.h>

#include "bytecode.h"
#include "runtimeEnv.h"
#include "runtimeValue.h"

#define VM_STACK_MAX 4096
#define VM_CALLSTACK_MAX 256
#define VM_FUNCTION_MAX 1024

typedef struct {
    char* name;
    int entry_ip;
    int end_ip;
    int param_count;
    char** param_names;
} VMFunction;

typedef struct {
    int return_ip;
    RuntimeEnvironment* env;
} VMCallFrame;

typedef struct {
    const BytecodeInstruction* code;
    size_t code_count;

    RuntimeValue stack[VM_STACK_MAX];
    int sp;

    VMCallFrame callstack[VM_CALLSTACK_MAX];
    int csp;

    VMFunction functions[VM_FUNCTION_MAX];
    int function_count;

    int ip;
    RuntimeEnvironment* global_env;
    RuntimeEnvironment* current_env;
} VM;

VM* vm_create(const BytecodeInstruction* code, size_t code_count);
void vm_destroy(VM* vm);
RuntimeValue vm_execute(VM* vm);

#endif // VM_H
