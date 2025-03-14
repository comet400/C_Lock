#include "runtimeEnv.h"
#include <string.h>
#include <stdio.h>
#include "lexer.h"
#include <sys/stat.h>
#include <windows.h>
#include <wchar.h>
#include <time.h>

#pragma warning(disable : 4996)

/***********************************************************
* Function: hash_string
* Description: this function hashes a string
* Parameters: const char* str
* Return: unsigned long
* ***********************************************************/
unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}




/***********************************************************
* Function: create_environment
* Description: this function creates a new runtime environment
* Parameters: RuntimeEnvironment* parent
* Return: RuntimeEnvironment*
***********************************************************/
RuntimeEnvironment* create_environment(RuntimeEnvironment* parent) {
    // Allocate memory for the runtime environment
    RuntimeEnvironment* env = (RuntimeEnvironment*)malloc(sizeof(RuntimeEnvironment));
    if (!env) {
        fprintf(stderr, "Memory allocation failed for RuntimeEnvironment\n");
        return NULL;
    }

    // Initialize fields
    env->function_returned = false;             // No function has returned yet
    env->return_value = make_null_value();      // Initialize return value as null
    env->parent = parent;                       // Link to the parent environment
    env->variables = NULL;                      // Initialize variable list to empty
    env->functions = NULL;                      // Initialize function list to empty

    return env;
}





/***********************************************************
* Function: strndump
* Description: this function duplicates a string
* Parameters: const char* src, size_t len
* Return: char*
* ***********************************************************/
RuntimeValue builtin_timestamp(RuntimeValue* args, size_t argc) {
    if (argc != 0) {
        fprintf(stderr, "Error: timestamp() expects no arguments.\n");
        return make_null_value();
    }

    time_t now = time(NULL);

    return make_int_value((long)now);
}




/***********************************************************
* Function: builtin_current_time
* Description: this function returns the current time
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_current_time(RuntimeValue* args, size_t argc) {
    if (argc != 0) {
        fprintf(stderr, "Error: current_time() expects no arguments.\n");
        return make_null_value();
    }

    time_t now = time(NULL);
    struct tm* localTime = localtime(&now);

    char buffer[9]; // HH:MM:SS
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
        localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

    return make_string_value(buffer);
}




/***********************************************************
* Function: builtin_current_date
* Description: this function returns the current date
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_current_date(RuntimeValue* args, size_t argc) {
    if (argc != 0) {
        fprintf(stderr, "Error: current_date() expects no arguments.\n");
        return make_null_value();
    }

    time_t now = time(NULL);
    struct tm* localTime = localtime(&now);

    char buffer[11]; // YYYY-MM-DD
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
        localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday);

    return make_string_value(buffer);
}




/***********************************************************
* Function: print_builtin
* Description: this function prints a runtime value
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue print_builtin(RuntimeValue* args, size_t argc) {
    for (size_t i = 0; i < argc; i++) {
        switch (args[i].type) {
        case RUNTIME_VALUE_INT:
            printf("%ld", args[i].int_val);
            break;
        case RUNTIME_VALUE_FLOAT:
            printf("%f", args[i].float_val);
            break;
        case RUNTIME_VALUE_BOOL:
            printf(args[i].bool_val ? "true" : "false");
            break;
        case RUNTIME_VALUE_STRING:
            printf("%s", args[i].string_val);
            break;
        case RUNTIME_VALUE_NULL:
            printf("null");
            break;
        default:
            printf("Unknown type");
            break;
        }
        if (i < argc - 1) {
            printf(", ");
        }
    }
    printf("\n");
    return make_null_value();
}




/***********************************************************
* Function: builtin_read_file
* Description: this function reads a file
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_read_file(RuntimeValue* args, size_t argc) {
    if (argc != 1 || args[0].type != RUNTIME_VALUE_STRING) {
        fprintf(stderr, "Error: read_file() expects a single string argument (file path).\n");
        return make_null_value();
    }

    const char* file_path = args[0].string_val;
    FILE* file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file '%s' for reading.\n", file_path);
        return make_null_value();
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate memory to read file content
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fprintf(stderr, "Error: Memory allocation failed while reading file.\n");
        fclose(file);
        return make_null_value();
    }

    fread(content, 1, file_size + 1, file);
    content[file_size] = '\0'; // Null-terminate the string
    fclose(file);

    RuntimeValue result = make_string_value(content);
    free(content); // Free temporary memory
    return result;
}




/***********************************************************
* Function: builtin_input
* Description: this function reads user input returns it as a string
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_input(RuntimeValue* args, size_t argc) {
    // Define a buffer to store user input
    char buffer[1024];

    // If arguments are provided, assume the first one is a string prompt
    if (argc > 0 && args[0].type == RUNTIME_VALUE_STRING) {
        printf("%s", args[0].string_val);
    }
    else if (argc > 0) {
        fprintf(stderr, "Error: input() expects a string as the first argument.\n");
        return make_null_value();
    }

    // Read a line from stdin
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        // Handle input errors (e.g., EOF or interrupt)
        fprintf(stderr, "Error: Failed to read input.\n");
        return make_null_value();
    }

    // Remove the newline character, if present
    buffer[strcspn(buffer, "\n")] = '\0';

    // Return the input as a string RuntimeValue
    return make_string_value(buffer);
}




/***********************************************************
* Function: builtin_write_file
* Description: this function writes to a file
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_write_file(RuntimeValue* args, size_t argc) {
    if (argc != 2 || args[0].type != RUNTIME_VALUE_STRING || args[1].type != RUNTIME_VALUE_STRING) {
        fprintf(stderr, "Error: write_file() expects two string arguments (file path, content).\n");
        return make_null_value();
    }

    const char* file_path = args[0].string_val;
    const char* content = args[1].string_val;
    FILE* file = fopen(file_path, "w");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file '%s' for writing.\n", file_path);
        return make_null_value();
    }

    fprintf(file, "%s", content);
    fclose(file);

    return make_bool_value(true); // Indicate success
}




/***********************************************************
* Function: builtin_append_file
* Description: this function appends to a file
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_append_file(RuntimeValue* args, size_t argc) {
    if (argc != 2 || args[0].type != RUNTIME_VALUE_STRING || args[1].type != RUNTIME_VALUE_STRING) {
        fprintf(stderr, "Error: append_file() expects two string arguments (file path, content).\n");
        return make_null_value();
    }

    const char* file_path = args[0].string_val;
    const char* content = args[1].string_val;
    FILE* file = fopen(file_path, "a");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file '%s' for appending.\n", file_path);
        return make_null_value();
    }

    fprintf(file, "%s", content);
    fclose(file);

    return make_bool_value(true); // Indicate success
}




/***********************************************************
* Function: builtin_file_exists
* Description: this function checks if a file exists
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_file_exists(RuntimeValue* args, size_t argc) {
    if (argc != 1 || args[0].type != RUNTIME_VALUE_STRING) {
        fprintf(stderr, "Error: file_exists() expects a single string argument (file path).\n");
        return make_null_value();
    }

    const char* file_path = args[0].string_val;
    struct stat buffer;
    int exists = stat(file_path, &buffer) == 0;

    return make_bool_value(exists);
}



/***********************************************************
* Function: builtin_file_size
* Description: this function returns the size of a file
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_file_size(RuntimeValue* args, size_t argc) {
    if (argc != 1 || args[0].type != RUNTIME_VALUE_STRING) {
        fprintf(stderr, "Error: file_size() expects a single string argument (file path).\n");
        return make_null_value();
    }

    const char* filePath = args[0].string_val;
    FILE* file = fopen(filePath, "rb");

    if (!file) {
        perror("Error opening file");
        return make_null_value();
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    if (size == -1L) {
        fprintf(stderr, "Error determining file size.\n");
        return make_null_value();
    }

    return make_int_value(size);
}




/***********************************************************
* Function: builtin_list_files
* Description: this function lists files in a directory windows only
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_list_files(RuntimeValue* args, size_t argc) {
    if (argc != 1 || args[0].type != RUNTIME_VALUE_STRING) {
        fprintf(stderr, "Error: list_files() expects a single string argument (directory path).\n");
        return make_null_value();
    }

    const char* dirPath = args[0].string_val;

    // Convert `dirPath` to a wide string
    size_t len = strlen(dirPath) + 3; // +3 for "\\*" and null terminator
    wchar_t* widePath = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (!widePath) {
        fprintf(stderr, "Error: Memory allocation failed for directory path.\n");
        return make_null_value();
    }

    mbstowcs(widePath, dirPath, len - 2);
    wcscat(widePath, L"\\*"); // Append "\\*" to match all files in the directory

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(widePath, &findFileData);

    free(widePath); // Free the allocated memory

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not open directory '%s'.\n", dirPath);
        return make_null_value();
    }

    printf("Files in '%s':\n", dirPath);
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            wprintf(L"  %s\n", findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return make_null_value();
}




/***********************************************************
* Function: builtin_delete_file
* Description: this function deletes a file
* Parameters: RuntimeValue* args, size_t argc
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue builtin_delete_file(RuntimeValue* args, size_t argc) {
    if (argc != 1 || args[0].type != RUNTIME_VALUE_STRING) {
        fprintf(stderr, "Error: delete_file() expects a single string argument (file path).\n");
        return make_null_value();
    }

    const char* filePath = args[0].string_val;
    if (remove(filePath) == 0) {
        printf("File '%s' deleted successfully.\n", filePath);
        return make_bool_value(true);
    }
    else {
        perror("Error deleting file");
        return make_bool_value(false);
    }
}




/***********************************************************
* Function: built_in_functions
* Description: this function adds built in functions to the environment
* Parameters: RuntimeEnvironment* env
* Return: RuntimeEnvironment*
* ***********************************************************/
RuntimeEnvironment* built_in_functions(RuntimeEnvironment* env) {
    if (!env) {
        fprintf(stderr, "Invalid environment provided to built_in_print.\n");
        return env;
    }

    RuntimeValue printFunction = make_builtin_function(print_builtin);
	env_set_func(env, "write", printFunction);

    RuntimeValue inputFunction = make_builtin_function(builtin_input);
	env_set_func(env, "input", inputFunction);

    add_time_built_ins(env);

    add_file_built_ins(env);

    return env;
}




/***********************************************************
* Function: add_time_built_ins
* Description: this function adds time built in functions to the environment
* Parameters: RuntimeEnvironment* env
* Return: void
* ***********************************************************/
void add_time_built_ins(RuntimeEnvironment* env)
{
    RuntimeValue timeFunction = make_builtin_function(builtin_current_time);
    env_set_func(env, "current_time", timeFunction);

    RuntimeValue dateFunction = make_builtin_function(builtin_current_date);
    env_set_func(env, "date_time", dateFunction);

    RuntimeValue timestampFunction = make_builtin_function(builtin_timestamp);
    env_set_func(env, "timestamp", timestampFunction);
}




/***********************************************************
* Function: add_file_built_ins
* Description: this function adds file built in functions to the environment
* Parameters: RuntimeEnvironment* env
* Return: void
* ***********************************************************/
void add_file_built_ins(RuntimeEnvironment* env)
{

    RuntimeValue readFunction = make_builtin_function(builtin_read_file);
	env_set_func(env, "read_file", readFunction);

    RuntimeValue writeFunction = make_builtin_function(builtin_write_file);
    env_set_func(env, "write_file", writeFunction);

    RuntimeValue appendFunction = make_builtin_function(builtin_append_file);
    env_set_func(env, "append_file", appendFunction);

    RuntimeValue existsFunction = make_builtin_function(builtin_file_exists);
    env_set_func(env, "file_exists", existsFunction);

    RuntimeValue sizeFunction = make_builtin_function(builtin_file_size);
    env_set_func(env, "file_size", sizeFunction);

    RuntimeValue listFunction = make_builtin_function(builtin_list_files);
    env_set_func(env, "list_files", listFunction);

    RuntimeValue deleteFunction = make_builtin_function(builtin_delete_file);
    env_set_func(env, "delete_file", deleteFunction);
}




/***********************************************************
* Function: free_runtime_value
* Description: this function frees the runtime value
* Parameters: RuntimeValue* val
* Return: Void
* ***********************************************************/
void free_runtime_value(RuntimeValue* val)
{
    if (!val) return;

    switch (val->type) {
    case RUNTIME_VALUE_STRING:
        if (val->string_val) {
            free(val->string_val);
            val->string_val = NULL;
        }
        break;
    case RUNTIME_VALUE_ARRAY:
        if (val->array_val.elements) {
            for (size_t i = 0; i < val->array_val.count; i++) {
                free_runtime_value(&val->array_val.elements[i]);
            }
            free(val->array_val.elements);
            val->array_val.elements = NULL;
        }
        break;
    case RUNTIME_VALUE_FUNCTION:
        if (val->function_val.env) {
            free_environment(val->function_val.env);
            val->function_val.env = NULL;
        }
        if (val->function_val.body) {
            free(val->function_val.body);
            val->function_val.body = NULL;
        }
        if (val->function_val.parameters) {
            free(val->function_val.parameters);
            val->function_val.parameters = NULL;
        }
        break;
    case RUNTIME_VALUE_SPECIAL:
        if (val->special_val) {
            free(val->special_val);
            val->special_val = NULL;
        }
        if (val->return_val) {
            free_runtime_value(val->return_val);
            free(val->return_val);
            val->return_val = NULL;
        }
        break;
    default:
        break;
    }
}




/***********************************************************
* Function: free_environment
* Description: this function frees the environment
* Parameters: RuntimeEnvironment* env
* Return: void
* ***********************************************************/
void free_environment(RuntimeEnvironment* env) {
    if (!env) return;

    if (env->variables) {
        EnvEntry* entry = env->variables;
        while (entry) {
            EnvEntry* next = entry->next;
            free(entry->key);
            free_runtime_value(&entry->value);
            free(entry);
            entry = next;
        }
    }

    if (env->functions) {
        EnvEntry* entry = env->functions;
        while (entry) {
            EnvEntry* next = entry->next;
            free(entry->key);
            free_runtime_value(&entry->value);
            free(entry);
            entry = next;
        }
    }
}
//manipulate the free functions very carefully





void env_set_var(RuntimeEnvironment* env, const char* key, RuntimeValue value) {
    if (!env || !key) {
        fprintf(stderr, "Invalid arguments provided to env_set_var.\n");
        return;
    }

    // Search for an existing variable in the current environment
    EnvEntry* entry = env->variables;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value; // Update value
            return;
        }
        entry = entry->next;
    }

    // Variable not found: Create a new entry
    EnvEntry* new_entry = (EnvEntry*)malloc(sizeof(EnvEntry));
    if (!new_entry) {
        fprintf(stderr, "Memory allocation failed for EnvEntry.\n");
        exit(EXIT_FAILURE);
    }

    new_entry->key = strdup(key); // Copy the key
    new_entry->value = value;
    new_entry->next = env->variables; // Insert at the head of the list
    env->variables = new_entry;
}




void env_set_func(RuntimeEnvironment* env, const char* key, RuntimeValue value) {
    if (!env || !key) {
        fprintf(stderr, "Invalid arguments provided to env_set_func.\n");
        return;
    }

    // Search for an existing function in the current environment
    EnvEntry* entry = env->functions;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value; // Update value
            return;
        }
        entry = entry->next;
    }

    // Function not found: Create a new entry
    EnvEntry* new_entry = (EnvEntry*)malloc(sizeof(EnvEntry));
    if (!new_entry) {
        fprintf(stderr, "Memory allocation failed for EnvEntry.\n");
        exit(EXIT_FAILURE);
    }

    new_entry->key = strdup(key); // Copy the key
    new_entry->value = value;
    new_entry->next = env->functions; // Insert at the head of the list
    env->functions = new_entry;
}







RuntimeValue env_get_var(RuntimeEnvironment* env, const char* key) {
    if (!env || !key) {
        return make_null_value();
    }

    RuntimeEnvironment* current = env;

    // Traverse the stack of environments
    while (current) {
        EnvEntry* entry = current->variables;
        while (entry) {
            if (strcmp(entry->key, key) == 0) {
                return entry->value; // Found variable
            }
            entry = entry->next;
        }
        current = current->parent; // Move to parent environment
    }

    // Variable not found
    return make_null_value();
}





RuntimeValue env_get_func(RuntimeEnvironment* env, const char* key) {
    if (!env || !key) {
        return make_null_value();
    }

    RuntimeEnvironment* current = env;

    // Traverse the stack of environments
    while (current) {
        EnvEntry* entry = current->functions;
        while (entry) {
            if (strcmp(entry->key, key) == 0) {
                return entry->value; // Found function
            }
            entry = entry->next;
        }
        current = current->parent; // Move to parent environment
    }

    // Function not found
    return make_null_value();
}




/***********************************************************
* Function: env_set_parent
* Description: this function sets the parent of a child environment
* Parameters: RuntimeEnvironment* child, RuntimeEnvironment* parent
* Return: void
* ***********************************************************/
void env_set_parent(RuntimeEnvironment* child, RuntimeEnvironment* parent) {
    if (child == NULL) {
        fprintf(stderr, "Error: Child environment is null.\n");
        return;
    }
    child->parent = parent;
}


