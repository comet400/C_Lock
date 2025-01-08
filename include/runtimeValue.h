
/**********************************************
* File: runtimeValue.h
* This file allows the creation of runtime values for the interpreter.
* the runtime values are used to store the values of variables and functions on our environment.
* This Code was written by Lukas Fukuoka Vieira.
* Contact : lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/




#pragma once

#ifndef RUNTIME_VALUE_H
#define RUNTIME_VALUE_H

#include <stdbool.h>
#include <stddef.h> // For size_t
#include "ast.h"

/**
 * An enum listing all possible runtime value types.
 */
typedef enum {
    RUNTIME_VALUE_INT,
    RUNTIME_VALUE_FLOAT,
    RUNTIME_VALUE_BOOL,
    RUNTIME_VALUE_STRING,
    RUNTIME_VALUE_NULL,
    RUNTIME_VALUE_BUILTIN,
    RUNTIME_VALUE_FUNCTION,
    RUNTIME_VALUE_SPECIAL,
    RUNTIME_VALUE_ARRAY
} RuntimeValueType;

struct RuntimeValue; // Forward declaration
/**
 * A struct that holds a tagged union for a runtime value.
 */
typedef struct RuntimeValue {
    RuntimeValueType type;
    union {
        long int_val;
        double float_val;
        bool bool_val;
        char* string_val;
        char* special_val;
        struct RuntimeValue* return_val;

        struct {
            struct RuntimeValue* elements;
            size_t count;
        } array_val;

        struct {
            struct RuntimeValue* (*fn)(struct RuntimeValue* args, size_t argc); // Function pointer
            struct RuntimeEnvironment* env;                                    // Environment for the function
            ASTNode* body;                                                     // User-defined function bod
            ASTNode* parameters;                                               // Parameters of the function
        } function_val;



        struct {
            struct RuntimeValue(*fn)(struct RuntimeValue* args, size_t argc); // Direct function pointer
        } builtin_val;
    };
} RuntimeValue;


/**
 * Create a runtime value of type int.
 */
RuntimeValue make_int_value(long i);

/**
 * Create a runtime value of type float.
 */
RuntimeValue make_float_value(double f);

/**
 * Create a runtime value of type bool.
 */
RuntimeValue make_bool_value(bool b);

/**
 * Create a runtime value of type string.
 * The string is copied into a newly allocated buffer.
 */
RuntimeValue make_string_value(const char* s);

/**
 * Create a runtime value of type null.
 */
RuntimeValue make_null_value(void);

/**
 * Create a runtime value for a built-in function.
 */
RuntimeValue make_builtin_function(
    struct RuntimeValue(*fn)(struct RuntimeValue* args, size_t argc)
);

void free_runtime_value(RuntimeValue* val);

#endif // RUNTIME_VALUE_H



