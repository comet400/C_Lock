/***********************************************************
* File: vm.c
* Stack-based bytecode VM for the Clock language.
* Executes BytecodeInstruction arrays produced by bytecode.c.
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub: https://github.com/comet400
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "runtimeEnv.h"
#include "runtimeValue.h"

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */

static void vm_push(VM* vm, RuntimeValue val) {
    if (vm->sp >= VM_STACK_MAX - 1) {
        fprintf(stderr, "VM: stack overflow\n");
        exit(EXIT_FAILURE);
    }
    vm->stack[++vm->sp] = val;
}

static RuntimeValue vm_pop(VM* vm) {
    if (vm->sp < 0) {
        fprintf(stderr, "VM: stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return vm->stack[vm->sp--];
}

static RuntimeValue vm_peek(VM* vm) {
    if (vm->sp < 0) {
        fprintf(stderr, "VM: stack underflow on peek\n");
        exit(EXIT_FAILURE);
    }
    return vm->stack[vm->sp];
}

/* Coerce a RuntimeValue to a C boolean (truthy). */
static bool vm_truthy(RuntimeValue v) {
    switch (v.type) {
        case RUNTIME_VALUE_BOOL:   return v.bool_val;
        case RUNTIME_VALUE_INT:    return v.int_val != 0;
        case RUNTIME_VALUE_FLOAT:  return v.float_val != 0.0;
        case RUNTIME_VALUE_STRING: return v.string_val && v.string_val[0] != '\0';
        case RUNTIME_VALUE_NULL:   return false;
        default:                   return true;
    }
}

/* Print a single RuntimeValue to stdout. */
static void vm_print_value(RuntimeValue v) {
    switch (v.type) {
        case RUNTIME_VALUE_INT:
            printf("%ld", v.int_val);
            break;
        case RUNTIME_VALUE_FLOAT:
            printf("%g", v.float_val);
            break;
        case RUNTIME_VALUE_BOOL:
            printf("%s", v.bool_val ? "true" : "false");
            break;
        case RUNTIME_VALUE_STRING:
            printf("%s", v.string_val ? v.string_val : "");
            break;
        case RUNTIME_VALUE_NULL:
            printf("null");
            break;
        case RUNTIME_VALUE_ARRAY:
            printf("[array(%zu)]", v.array_val.count);
            break;
        default:
            printf("<value>");
            break;
    }
}

/* Read a VMFunction by name; returns NULL if not found. */
static VMFunction* vm_find_function(VM* vm, const char* name) {
    for (int i = 0; i < vm->function_count; i++) {
        if (vm->functions[i].name && strcmp(vm->functions[i].name, name) == 0) {
            return &vm->functions[i];
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Arithmetic helpers                                                 */
/* ------------------------------------------------------------------ */

static RuntimeValue vm_arith(RuntimeValue a, RuntimeValue b, BytecodeOpcode op) {
    /* Float promotion */
    if (a.type == RUNTIME_VALUE_FLOAT || b.type == RUNTIME_VALUE_FLOAT) {
        double fa = (a.type == RUNTIME_VALUE_FLOAT) ? a.float_val : (double)a.int_val;
        double fb = (b.type == RUNTIME_VALUE_FLOAT) ? b.float_val : (double)b.int_val;
        switch (op) {
            case OP_ADD_:      return make_float_value(fa + fb);
            case OP_SUBTRACT:  return make_float_value(fa - fb);
            case OP_MULTIPLY:  return make_float_value(fa * fb);
            case OP_DIVIDE:
                if (fb == 0.0) { fprintf(stderr, "VM: division by zero\n"); exit(EXIT_FAILURE); }
                return make_float_value(fa / fb);
            default: break;
        }
    }

    /* String concatenation on + */
    if (op == OP_ADD_ && a.type == RUNTIME_VALUE_STRING && b.type == RUNTIME_VALUE_STRING) {
        size_t la = strlen(a.string_val ? a.string_val : "");
        size_t lb = strlen(b.string_val ? b.string_val : "");
        char* buf = (char*)malloc(la + lb + 1);
        if (!buf) { fprintf(stderr, "VM: malloc failed\n"); exit(EXIT_FAILURE); }
        strcpy(buf, a.string_val ? a.string_val : "");
        strcat(buf, b.string_val ? b.string_val : "");
        RuntimeValue r = make_string_value(buf);
        free(buf);
        return r;
    }

    long ia = (a.type == RUNTIME_VALUE_INT) ? a.int_val : (long)a.float_val;
    long ib = (b.type == RUNTIME_VALUE_INT) ? b.int_val : (long)b.float_val;
    switch (op) {
        case OP_ADD_:      return make_int_value(ia + ib);
        case OP_SUBTRACT:  return make_int_value(ia - ib);
        case OP_MULTIPLY:  return make_int_value(ia * ib);
        case OP_DIVIDE:
            if (ib == 0) { fprintf(stderr, "VM: division by zero\n"); exit(EXIT_FAILURE); }
            return make_int_value(ia / ib);
        case OP_MODULO:
        case OP_MODULO_EQUAL:
            if (ib == 0) { fprintf(stderr, "VM: modulo by zero\n"); exit(EXIT_FAILURE); }
            return make_int_value(ia % ib);
        default: break;
    }
    return make_null_value();
}

static RuntimeValue vm_compare(RuntimeValue a, RuntimeValue b, BytecodeOpcode op) {
    /* numeric comparison */
    if ((a.type == RUNTIME_VALUE_INT || a.type == RUNTIME_VALUE_FLOAT) &&
        (b.type == RUNTIME_VALUE_INT || b.type == RUNTIME_VALUE_FLOAT)) {
        double fa = (a.type == RUNTIME_VALUE_FLOAT) ? a.float_val : (double)a.int_val;
        double fb = (b.type == RUNTIME_VALUE_FLOAT) ? b.float_val : (double)b.int_val;
        switch (op) {
            case OP_EQUAL:         return make_bool_value(fa == fb);
            case OP_NOT_EQUAL:     return make_bool_value(fa != fb);
            case OP_LESS:          return make_bool_value(fa < fb);
            case OP_GREATER:       return make_bool_value(fa > fb);
            case OP_LESS_EQUAL:    return make_bool_value(fa <= fb);
            case OP_GREATER_EQUAL: return make_bool_value(fa >= fb);
            default: break;
        }
    }
    /* string comparison */
    if (a.type == RUNTIME_VALUE_STRING && b.type == RUNTIME_VALUE_STRING) {
        int c = strcmp(a.string_val ? a.string_val : "",
                       b.string_val ? b.string_val : "");
        switch (op) {
            case OP_EQUAL:         return make_bool_value(c == 0);
            case OP_NOT_EQUAL:     return make_bool_value(c != 0);
            case OP_LESS:          return make_bool_value(c < 0);
            case OP_GREATER:       return make_bool_value(c > 0);
            case OP_LESS_EQUAL:    return make_bool_value(c <= 0);
            case OP_GREATER_EQUAL: return make_bool_value(c >= 0);
            default: break;
        }
    }
    /* bool / null equality */
    if (op == OP_EQUAL) {
        if (a.type == RUNTIME_VALUE_BOOL && b.type == RUNTIME_VALUE_BOOL)
            return make_bool_value(a.bool_val == b.bool_val);
        if (a.type == RUNTIME_VALUE_NULL && b.type == RUNTIME_VALUE_NULL)
            return make_bool_value(true);
    }
    if (op == OP_NOT_EQUAL) {
        return make_bool_value(!(vm_truthy(vm_compare(a, b, OP_EQUAL))));
    }
    return make_bool_value(false);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

VM* vm_create(const BytecodeInstruction* code, size_t code_count) {
    VM* vm = (VM*)calloc(1, sizeof(VM));
    if (!vm) {
        fprintf(stderr, "VM: failed to allocate VM\n");
        exit(EXIT_FAILURE);
    }
    vm->code = code;
    vm->code_count = code_count;
    vm->sp = -1;
    vm->csp = -1;
    vm->ip = 0;
    vm->function_count = 0;
    vm->global_env = create_environment(NULL);
    vm->global_env = built_in_functions(vm->global_env);
    vm->current_env = vm->global_env;
    return vm;
}

void vm_destroy(VM* vm) {
    if (!vm) return;
    free_environment(vm->global_env);
    free(vm);
}

/* ------------------------------------------------------------------ */
/*  First-pass: register all OP_DECL_FUNCTION into the function table */
/* ------------------------------------------------------------------ */

static void vm_register_functions(VM* vm) {
    for (size_t i = 0; i < vm->code_count; i++) {
        const BytecodeInstruction* instr = &vm->code[i];
        if (instr->opcode == OP_DECL_FUNCTION) {
            if (vm->function_count >= VM_FUNCTION_MAX) {
                fprintf(stderr, "VM: too many functions\n");
                exit(EXIT_FAILURE);
            }
            VMFunction* fn = &vm->functions[vm->function_count++];
            fn->name        = instr->operand.function_decl.func_name;
            fn->entry_ip    = instr->operand.function_decl.body_index;
            fn->end_ip      = instr->operand.function_decl.end_index;
            fn->param_count = instr->operand.function_decl.param_count;
            fn->param_names = instr->operand.function_decl.param_names;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Main execution loop                                                */
/* ------------------------------------------------------------------ */

RuntimeValue vm_execute(VM* vm) {
    /* Register all declared functions before running top-level code. */
    vm_register_functions(vm);

    while (vm->ip < (int)vm->code_count) {
        const BytecodeInstruction* instr = &vm->code[vm->ip];
        vm->ip++;

        switch (instr->opcode) {

        /* ---- Push constants ---- */
        case OP_PUSH_INT:
            vm_push(vm, make_int_value((long)instr->operand.int_operand));
            break;

        case OP_PUSH_FLOAT:
            vm_push(vm, make_float_value((double)instr->operand.float_operand));
            break;

        case OP_PUSH_BOOL:
            vm_push(vm, make_bool_value(instr->operand.bool_operand));
            break;

        case OP_PUSH_STRING:
            vm_push(vm, make_string_value(instr->operand.string_operand));
            break;

        /* ---- Variables ---- */
        case OP_LOAD_VAR_: {
            const char* name = instr->operand.string_operand;
            RuntimeValue v = env_get_var(vm->current_env, name);
            vm_push(vm, v);
            break;
        }

        case OP_STORE_VAR_: {
            const char* name = instr->operand.string_operand;
            RuntimeValue v = vm_pop(vm);
            env_set_var(vm->current_env, name, v);
            break;
        }

        case OP_ASSIGN: {
            /* same as STORE but uses string_operand */
            RuntimeValue v = vm_pop(vm);
            env_set_var(vm->current_env, instr->operand.string_operand, v);
            break;
        }

        /* ---- Arithmetic ---- */
        case OP_ADD_:
        case OP_SUBTRACT:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_MODULO:
        case OP_MODULO_EQUAL: {
            RuntimeValue b = vm_pop(vm);
            RuntimeValue a = vm_pop(vm);
            vm_push(vm, vm_arith(a, b, instr->opcode));
            break;
        }

        /* ---- Unary ---- */
        case OP_NEGATE: {
            RuntimeValue a = vm_pop(vm);
            if (a.type == RUNTIME_VALUE_INT)   vm_push(vm, make_int_value(-a.int_val));
            else if (a.type == RUNTIME_VALUE_FLOAT) vm_push(vm, make_float_value(-a.float_val));
            else vm_push(vm, make_null_value());
            break;
        }

        case OP_NOT_: {
            RuntimeValue a = vm_pop(vm);
            vm_push(vm, make_bool_value(!vm_truthy(a)));
            break;
        }

        /* ---- Comparison ---- */
        case OP_EQUAL:
        case OP_NOT_EQUAL:
        case OP_LESS:
        case OP_GREATER:
        case OP_LESS_EQUAL:
        case OP_GREATER_EQUAL: {
            RuntimeValue b = vm_pop(vm);
            RuntimeValue a = vm_pop(vm);
            vm_push(vm, vm_compare(a, b, instr->opcode));
            break;
        }

        /* ---- Logical ---- */
        case OP_AND_: {
            RuntimeValue b = vm_pop(vm);
            RuntimeValue a = vm_pop(vm);
            vm_push(vm, make_bool_value(vm_truthy(a) && vm_truthy(b)));
            break;
        }

        case OP_OR_: {
            RuntimeValue b = vm_pop(vm);
            RuntimeValue a = vm_pop(vm);
            vm_push(vm, make_bool_value(vm_truthy(a) || vm_truthy(b)));
            break;
        }

        /* ---- Jumps ---- */
        case OP_JUMP_TO:
            vm->ip = instr->operand.int_operand;
            break;

        case OP_JUMP_TO_IF_FALSE: {
            RuntimeValue cond = vm_pop(vm);
            if (!vm_truthy(cond)) {
                vm->ip = instr->operand.int_operand;
            }
            break;
        }

        case OP_JUMP_IF_FALSE_: {
            RuntimeValue cond = vm_peek(vm);
            if (!vm_truthy(cond)) {
                vm->ip = instr->operand.int_operand;
            }
            break;
        }

        case OP_JUMP_IF_TRUE_: {
            RuntimeValue cond = vm_peek(vm);
            if (vm_truthy(cond)) {
                vm->ip = instr->operand.int_operand;
            }
            break;
        }

        case OP_JUMP_:
            vm->ip = instr->operand.jump.target_index;
            break;

        /* ---- Pop ---- */
        case OP_POP:
            if (vm->sp >= 0) vm_pop(vm);
            break;

        /* ---- Print / Input ---- */
        case OP_PRINT_: {
            RuntimeValue v = vm_pop(vm);
            vm_print_value(v);
            printf("\n");
            break;
        }

        case OP_INPUT_: {
            char buf[1024];
            if (fgets(buf, sizeof(buf), stdin)) {
                buf[strcspn(buf, "\n")] = '\0';
                vm_push(vm, make_string_value(buf));
            } else {
                vm_push(vm, make_null_value());
            }
            break;
        }

        /* ---- Array ---- */
        case OP_ARRAY_SET_: {
            /*
             * Generator emits this after N element pushes for array literals.
             * count tells how many stack values to collect.
             */
            int count = instr->operand.array_literal.count;
            RuntimeValue arr;
            arr.type = RUNTIME_VALUE_ARRAY;
            arr.array_val.count = (size_t)count;
            arr.array_val.elements = (RuntimeValue*)malloc(sizeof(RuntimeValue) * (size_t)count);
            if (!arr.array_val.elements) {
                fprintf(stderr, "VM: malloc failed for array\n");
                exit(EXIT_FAILURE);
            }
            for (int k = count - 1; k >= 0; k--) {
                arr.array_val.elements[k] = vm_pop(vm);
            }
            vm_push(vm, arr);
            break;
        }

        case OP_ARRAY_GET_: {
            RuntimeValue idx_val = vm_pop(vm);
            RuntimeValue arr_val = vm_pop(vm);
            if (arr_val.type != RUNTIME_VALUE_ARRAY) {
                fprintf(stderr, "VM: OP_ARRAY_GET_ on non-array\n");
                vm_push(vm, make_null_value());
                break;
            }
            long idx = (idx_val.type == RUNTIME_VALUE_INT) ? idx_val.int_val : (long)idx_val.float_val;
            if (idx < 0 || idx >= (long)arr_val.array_val.count) {
                fprintf(stderr, "VM: array index out of bounds (%ld)\n", idx);
                vm_push(vm, make_null_value());
                break;
            }
            vm_push(vm, arr_val.array_val.elements[idx]);
            break;
        }

        /* ---- Function declaration ---- */
        case OP_DECL_FUNCTION: {
            /*
             * We already registered all functions in vm_register_functions.
             * At runtime we just skip past the function body.
             */
            int end = instr->operand.function_decl.end_index;
            if (end > 0) {
                vm->ip = end;
            }
            break;
        }

        case OP_END_FUNCTION:
            /* Should only be reached if something jumped here incorrectly; treat as halt. */
            goto vm_done;

        /* ---- Function call ---- */
        case OP_CALL_FUNCTION: {
            const char* fname = instr->operand.call.func_name;
            int arg_count     = instr->operand.call.arg_count;

            /* Check for builtin first */
            RuntimeValue fn_val = env_get_func(vm->current_env, fname);

            if (fn_val.type == RUNTIME_VALUE_BUILTIN) {
                /* Collect args from stack (pushed left-to-right, so reverse). */
                RuntimeValue* args = (RuntimeValue*)malloc(sizeof(RuntimeValue) * (size_t)(arg_count > 0 ? arg_count : 1));
                if (!args) { fprintf(stderr, "VM: malloc failed\n"); exit(EXIT_FAILURE); }
                for (int k = arg_count - 1; k >= 0; k--) {
                    args[k] = vm_pop(vm);
                }
                RuntimeValue result = fn_val.builtin_val.fn(args, (size_t)arg_count);
                free(args);
                vm_push(vm, result);
                break;
            }

            /* User-defined function */
            VMFunction* fn = vm_find_function(vm, fname);
            if (!fn) {
                fprintf(stderr, "VM: undefined function '%s'\n", fname ? fname : "(null)");
                vm_push(vm, make_null_value());
                break;
            }

            /* Collect args */
            RuntimeValue* args = (RuntimeValue*)malloc(sizeof(RuntimeValue) * (size_t)(arg_count > 0 ? arg_count : 1));
            if (!args) { fprintf(stderr, "VM: malloc failed\n"); exit(EXIT_FAILURE); }
            for (int k = arg_count - 1; k >= 0; k--) {
                args[k] = vm_pop(vm);
            }

            /* Push call frame */
            if (vm->csp >= VM_CALLSTACK_MAX - 1) {
                fprintf(stderr, "VM: call stack overflow\n");
                exit(EXIT_FAILURE);
            }
            vm->csp++;
            vm->callstack[vm->csp].return_ip  = vm->ip;
            vm->callstack[vm->csp].env        = vm->current_env;

            /* New environment for the function, child of global */
            RuntimeEnvironment* fn_env = create_environment(vm->global_env);
            vm->current_env = fn_env;

            /* Bind parameters */
            for (int k = 0; k < fn->param_count && k < arg_count; k++) {
                if (fn->param_names && fn->param_names[k]) {
                    env_set_var(fn_env, fn->param_names[k], args[k]);
                }
            }
            free(args);

            /* Jump to function body */
            vm->ip = fn->entry_ip;
            break;
        }

        /* ---- Return ---- */
        case OP_RETURN_:
        case OP_RETURN_STM: {
            RuntimeValue ret = (vm->sp >= 0) ? vm_pop(vm) : make_null_value();

            if (vm->csp < 0) {
                /* Top-level return */
                vm_push(vm, ret);
                goto vm_done;
            }

            /* Restore caller state */
            free_environment(vm->current_env);
            vm->current_env = vm->callstack[vm->csp].env;
            vm->ip          = vm->callstack[vm->csp].return_ip;
            vm->csp--;

            vm_push(vm, ret);
            break;
        }

        /* ---- Break / Continue (treated as jumps in loop context) ---- */
        case OP_BREAK_:
        case OP_CONTINUE_:
            /* These are handled by the jump placeholders; nothing to do at VM level. */
            break;

        /* ---- Switch / When ---- */
        case OP_SWITCH_:
            /* Handled inline by the when/equal checks in bytecode. */
            break;

        case OP_WHEN_: {
            RuntimeValue cond = vm_pop(vm);
            if (!vm_truthy(cond)) {
                vm->ip = instr->operand.when.body_index;
            }
            break;
        }

        case OP_DEFAULT_:
            break;

        /* ---- Halt ---- */
        case OP_HALT:
            goto vm_done;

        /* ---- Ignored / legacy opcodes ---- */
        case OP_CALL_:
        case OP_LOAD_CONST_:
        case OP_PARAMETER_LIST_:
        case OP_SCOPE:
            break;

        default:
            fprintf(stderr, "VM: unknown opcode %d at ip=%d\n", instr->opcode, vm->ip - 1);
            break;
        }
    }

vm_done:
    return (vm->sp >= 0) ? vm_peek(vm) : make_null_value();
}
