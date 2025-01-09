
/**********************************************
* File: runtimeEnv.h
* This file have the runtime environment for the interpreter.
* the environment is responsible for storing variables and functions.
* This Code was written by Lukas Fukuoka Vieira.
* Contact : lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/



#pragma once
#pragma once
#ifndef RUNTIME_ENV_H
#define RUNTIME_ENV_H

#include <stdlib.h>
#include <string.h>
#include "runtimeValue.h"

/**
 * A simple environment entry: key = variable name, value = stored RuntimeValue.
 */
typedef struct EnvEntry {
    char* key;
    RuntimeValue value; // Use RuntimeValue directly
    struct EnvEntry* next;
} EnvEntry;

/**
 * The environment (or context) with a hash table of EnvEntries.
 */
 typedef struct RuntimeEnvironment {
    size_t capacity; // The number of buckets in the hash table
    EnvEntry** table; // An array of pointers to EnvEntry
    struct RuntimeEnvironment* parent;
	bool function_returned; // Flag to indicate if a function has returned
	RuntimeValue return_value; // The value returned by a function
 } RuntimeEnvironment;

/**
 * Initialize an environment with a given capacity (e.g., 128 for MVP and testing).
 */
RuntimeEnvironment* create_environment(size_t capacity);

/**
 * Free the entire environment, including all linked EnvEntry nodes and any allocated keys/values.
 */
void free_environment(RuntimeEnvironment* env);


/**
 * Simple hashing function for strings.
 */
unsigned long hash_string(const char* str);

/**
 * Set or update a variable in the environment.
 * - If the key already exists, updates the value.
 * - If the key doesn't exist, creates a new entry.
 */
void env_set(RuntimeEnvironment* env, const char* key, RuntimeValue value);

/**
 * Retrieve a variable from the environment.
 * - If the key exists, returns the associated RuntimeValue.
 * - If the key doesn't exist, returns a null value.
 */
RuntimeValue env_get(RuntimeEnvironment* env, const char* key);

/**
 * Built-in function: some of the built in functions for our testing enviroment
 */
RuntimeEnvironment* built_in_functions(RuntimeEnvironment* env);

RuntimeValue builtin_input(RuntimeValue* args, size_t argc);

RuntimeValue builtin_read_file(RuntimeValue* args, size_t argc);

RuntimeValue builtin_file_exists(RuntimeValue* args, size_t argc);

RuntimeValue builtin_append_file(RuntimeValue* args, size_t argc);

RuntimeValue builtin_write_file(RuntimeValue* args, size_t argc);

RuntimeValue builtin_delete_file(RuntimeValue* args, size_t argc);

RuntimeValue builtin_list_files(RuntimeValue* args, size_t argc);

RuntimeValue builtin_file_size(RuntimeValue* args, size_t argc);

RuntimeValue builtin_current_time(RuntimeValue* args, size_t argc);

RuntimeValue builtin_timestamp(RuntimeValue* args, size_t argc);

RuntimeValue builtin_current_date(RuntimeValue* args, size_t argc);

void add_time_built_ins(RuntimeEnvironment* env);

void add_file_built_ins(RuntimeEnvironment* env);

void env_set_parent(RuntimeEnvironment* child, RuntimeEnvironment* parent);



/**
 * A built-in print function to print arguments of various types.
 */
RuntimeValue print_builtin(RuntimeValue* args, size_t argc);

#endif // RUNTIME_ENV
