#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "bytecode.h"
#include "vm.h"
#include "aot.h"

#define INITIAL_BUFFER_SIZE 1024

// Cross-platform terminal helpers
#ifdef _WIN32
#define CLEAR_SCREEN() system("cls")
#define SET_COLOR_GREEN() system("color 0A")
#else
#define CLEAR_SCREEN() printf("\033[2J\033[H")
#define SET_COLOR_GREEN() printf("\033[0;32m")
#endif



// A small helper to print the tokens for debugging
void print_tokens(const TokenArray* tokens) {
    printf("=== TOKENS ===\n");
    for (size_t i = 0; i < tokens->size; i++) {
        const Token* t = &tokens->data[i];
        // Token info: type, value, line/column
        printf("[%3zu] Type = %d, Value = '%s', Line = %zu, Col = %zu\n",
            i, t->type, t->value ? t->value : "",
            t->line, t->column);
    }
    printf("=== END TOKENS ===\n\n");
}

void init_interpreter()
{
    bool debug = false;
    char* fullCode = (char*)"";
    CLEAR_SCREEN();
    // 1) Prompt user for input
    //change print color to green
    SET_COLOR_GREEN();

    size_t bufferSize = INITIAL_BUFFER_SIZE;
    size_t currentLength = 0;
    char* sourceCode = (char*)malloc(bufferSize);
    if (!sourceCode) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    while (1) {
        printf("%s\n", fullCode);
        printf(">> "); // Use '\r' to overwrite the current line
        fflush(stdout);  // Ensure the prompt is displayed immediately

        char line[256];
        if (!fgets(line, sizeof(line), stdin)) {
            break; // Stop on input error
        }

        // Check for 'END' to stop input
        if (strncmp(line, "END", 3) == 0 && (line[3] == '\n' || line[3] == '\0')) {
            printf("\033[0;36m\n");
            break;
        }
        else if (strncmp(line, "DEBUG", 5) == 0 && (line[5] == '\n' || line[5] == '\0')) {
            printf("\033[0;36m\n");
            debug = true;
            break;
        }

        size_t lineLen = strlen(line);
        // Resize buffer if needed
        if (currentLength + lineLen + 1 > bufferSize) {
            bufferSize *= 2;
            char* newBuffer = (char*)realloc(sourceCode, bufferSize);
            if (!newBuffer) {
                fprintf(stderr, "Memory allocation failed during buffer resize.\n");
                free(sourceCode);
                exit(1);
            }
            sourceCode = newBuffer;
        }

        // Append line to sourceCode
        strcpy(sourceCode + currentLength, line);
        currentLength += lineLen;
        fullCode = sourceCode;
        CLEAR_SCREEN();
    }

    // Null-terminate the string
    sourceCode[currentLength] = '\0';


    // Null-terminate the string
    sourceCode[currentLength] = '\0';

    if (currentLength == 0) {
        fprintf(stderr, "No input provided.\n");
        printf("\033[0;37m");
        free(sourceCode);
        return;
    }

    // 2) Tokenize the source string
    printf("Program Output: \n\n");
    TokenArray tokens = tokenize(sourceCode);

    if (debug) print_tokens(&tokens);

    // 3) Create a parser and parse into an AST
    Parser parser = create_parser(&tokens);
    ASTNode* root = parse_program(&parser);

    if (debug) print_ast(root, 0);

    // 4) Generate bytecode and execute via VM
    BytecodeInstruction* bytecode = malloc(sizeof(BytecodeInstruction) * 1024);
    size_t bytecode_count = 0;
    size_t bytecode_capacity = 1024;
    if (!bytecode) { fprintf(stderr, "Memory allocation failed.\n"); return; }
    generate_bytecode(root, &bytecode, &bytecode_count, &bytecode_capacity);

    if (debug) print_byteCode(bytecode, bytecode_count);

    VM* vm = vm_create(bytecode, bytecode_count);
    vm_execute(vm);
    vm_destroy(vm);
    free(bytecode);


    // 5) Clean up: free AST, tokens, etc.


    free_ast_node(root);
    free_token_array(&tokens);
    free(sourceCode);


    //go back to white
    printf("\033[0;37m");
}

static char* read_file_to_buffer(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length < 0) {
        fclose(file);
        return NULL;
    }

    char* sourceCode = (char*)malloc((size_t)length + 1);
    if (!sourceCode) {
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(sourceCode, 1, (size_t)length, file);
    fclose(file);

    if (bytesRead != (size_t)length) {
        free(sourceCode);
        return NULL;
    }

    sourceCode[length] = '\0';
    return sourceCode;
}

static int run_file_with_vm(const char* filename) {
    char* sourceCode = read_file_to_buffer(filename);
    if (!sourceCode) {
        perror("Error opening/reading file");
        return 1;
    }

    printf("\033[0;36m\n");
    printf("Program Output: \n\n");

    TokenArray tokens = tokenize(sourceCode);
    Parser parser = create_parser(&tokens);
    ASTNode* root = parse_program(&parser);

    BytecodeInstruction* bytecode = malloc(sizeof(BytecodeInstruction) * 1024);
    size_t bytecode_count = 0;
    size_t bytecode_capacity = 1024;
    if (!bytecode) {
        fprintf(stderr, "Memory allocation failed.\n");
        free_ast_node(root);
        free_token_array(&tokens);
        free(sourceCode);
        return 1;
    }

    generate_bytecode(root, &bytecode, &bytecode_count, &bytecode_capacity);

    VM* vm = vm_create(bytecode, bytecode_count);
    vm_execute(vm);
    vm_destroy(vm);

    free(bytecode);
    free_ast_node(root);
    free_token_array(&tokens);
    free(sourceCode);
    return 0;
}

static int compile_file_aot(const char* input_file, const char* output_binary) {
    char* sourceCode = read_file_to_buffer(input_file);
    if (!sourceCode) {
        perror("Error opening/reading file");
        return 1;
    }

    TokenArray tokens = tokenize(sourceCode);
    Parser parser = create_parser(&tokens);
    ASTNode* root = parse_program(&parser);

    char err[512] = {0};
    bool ok = aot_compile_ast(root, output_binary, err, sizeof(err));

    free_ast_node(root);
    free_token_array(&tokens);
    free(sourceCode);

    if (!ok) {
        fprintf(stderr, "%s\n", err[0] ? err : "AOT compile failed");
        return 1;
    }

    printf("AOT build complete: %s\n", output_binary);
    return 0;
}

static int run_source_with_aot(const char* sourceCode, bool debug) {
    TokenArray tokens = tokenize(sourceCode);
    if (debug) print_tokens(&tokens);

    Parser parser = create_parser(&tokens);
    ASTNode* root = parse_program(&parser);
    if (debug) print_ast(root, 0);

    char output_binary[256];
    snprintf(output_binary, sizeof(output_binary), "/tmp/cllc_aot_repl_%lu", (unsigned long)time(NULL));

    char err[512] = {0};
    bool ok = aot_compile_ast(root, output_binary, err, sizeof(err));

    if (!ok) {
        fprintf(stderr, "%s\n", err[0] ? err : "AOT compile failed");
        free_ast_node(root);
        free_token_array(&tokens);
        return 1;
    }

    char run_cmd[512];
#ifdef _WIN32
    snprintf(run_cmd, sizeof(run_cmd), "\"%s\"", output_binary);
#else
    snprintf(run_cmd, sizeof(run_cmd), "\"%s\"", output_binary);
#endif
    int rc = system(run_cmd);

    char c_output[300];
    snprintf(c_output, sizeof(c_output), "%s.c", output_binary);
    remove(c_output);
    remove(output_binary);

    free_ast_node(root);
    free_token_array(&tokens);

    return rc == 0 ? 0 : 1;
}

static void init_interpreter_aot()
{
    bool debug = false;
    char* fullCode = (char*)"";
    CLEAR_SCREEN();
    SET_COLOR_GREEN();

    size_t bufferSize = INITIAL_BUFFER_SIZE;
    size_t currentLength = 0;
    char* sourceCode = (char*)malloc(bufferSize);
    if (!sourceCode) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    while (1) {
        printf("%s\n", fullCode);
        printf(">> ");
        fflush(stdout);

        char line[256];
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        if (strncmp(line, "END", 3) == 0 && (line[3] == '\n' || line[3] == '\0')) {
            printf("\033[0;36m\n");
            break;
        }
        else if (strncmp(line, "DEBUG", 5) == 0 && (line[5] == '\n' || line[5] == '\0')) {
            printf("\033[0;36m\n");
            debug = true;
            break;
        }

        size_t lineLen = strlen(line);
        if (currentLength + lineLen + 1 > bufferSize) {
            bufferSize *= 2;
            char* newBuffer = (char*)realloc(sourceCode, bufferSize);
            if (!newBuffer) {
                fprintf(stderr, "Memory allocation failed during buffer resize.\n");
                free(sourceCode);
                exit(1);
            }
            sourceCode = newBuffer;
        }

        strcpy(sourceCode + currentLength, line);
        currentLength += lineLen;
        fullCode = sourceCode;
        CLEAR_SCREEN();
    }

    sourceCode[currentLength] = '\0';

    if (currentLength == 0) {
        fprintf(stderr, "No input provided.\n");
        printf("\033[0;37m");
        free(sourceCode);
        return;
    }

    printf("Program Output: \n\n");
    run_source_with_aot(sourceCode, debug);

    free(sourceCode);
    printf("\033[0;37m");
}


int main(int argc, char* argv[]) {
    if (argc == 4 && strcmp(argv[1], "--aot") == 0) {
        return compile_file_aot(argv[2], argv[3]);
    }

    // cllc --aot <file>  → derive output name from input (strip .clk or append _native)
    if (argc == 3 && strcmp(argv[1], "--aot") == 0) {
        const char* input = argv[2];
        char output[512];
        size_t len = strlen(input);
        if (len > 4 && strcmp(input + len - 4, ".clk") == 0) {
            snprintf(output, sizeof(output), "%.*s", (int)(len - 4), input);
        } else {
            snprintf(output, sizeof(output), "%s_native", input);
        }
        return compile_file_aot(input, output);
    }

    if (argc == 2 && strcmp(argv[1], "--aot") == 0) {
        init_interpreter_aot();
        getchar();
        return 0;
    }

    if (argc == 2) {
        return run_file_with_vm(argv[1]);
    }
    else {
        // Interactive mode
        init_interpreter();
        getchar();
    }

    return 0;
}









