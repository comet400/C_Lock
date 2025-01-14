#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "bytecode.h"

#pragma warning(disable : 4996) 

#define INITIAL_BUFFER_SIZE 1024

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

    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
    // 1) Prompt user for input
    //change print color to green
    #ifdef _WIN32
    system("color 0A");
#else
    printf("\033[0;32m");  // Set text color to green
    fflush(stdout);        // Flush the output buffer
#endif


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
        #if defined(_WIN32) || defined(__STDC_LIB_EXT1__)
        strcpy_s(sourceCode + currentLength, bufferSize - currentLength, line);
        #else
        strncpy(sourceCode + currentLength, line, bufferSize - currentLength);
        sourceCode[bufferSize - 1] = '\0';
        #endif

        currentLength += lineLen;
        fullCode = sourceCode;
        
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif
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

    ASTNode** flat_list = NULL;
    size_t flat_count = 0;
    flatten_ast(root, &flat_list, &flat_count);

    if (debug) print_flattened_ast(flat_list, flat_count);
    
    // 4) Interpret (execute) the AST
    interpret(root);


    // 5) Clean up: free AST, tokens, etc.
    BytecodeInstruction* bytecode = malloc(sizeof(BytecodeInstruction) * 1024);
    size_t bytecode_count = 0;
    size_t bytecode_capacity = 1024;
    generate_bytecode(root, &bytecode, &bytecode_count, &bytecode_capacity);
    if (debug) print_byteCode(bytecode, bytecode_count);
    free(bytecode);
    
    free_ast_node(root);
    free_token_array(&tokens);
    free(sourceCode);
    free(flat_list);

    //go back to white
    printf("\033[0;37m");
}


int main(int argc, char* argv[]) {
    if (argc == 2) {
        // File mode
        const char* filename = argv[1];
        FILE* file = fopen(filename, "rb");  // Open in binary mode
        if (!file) {
            perror("Error opening file");
            return 1;
        }

        // Get file size
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate memory for file content (including null terminator)
        char* sourceCode = (char*)malloc(length + 1);
        if (!sourceCode) {
            fprintf(stderr, "Memory allocation failed.\n");
            fclose(file);
            return 1;
        }

        // Read file content
        size_t bytesRead = fread(sourceCode, 1, length+1, file);
        fclose(file);

        if (bytesRead != length) {
            fprintf(stderr, "Error reading file: expected %ld bytes, got %zu bytes.\n", length, bytesRead);
            free(sourceCode);
            return 1;
        }

        // Null-terminate the string
        sourceCode[length] = '\0';

        printf("\033[0;36m\n");
        printf("Program Output: \n\n");

        TokenArray tokens = tokenize(sourceCode);

        Parser parser = create_parser(&tokens);
        ASTNode* root = parse_program(&parser);

        interpret(root);

        // Clean up
        free_ast_node(root);
        free_token_array(&tokens);
        free(sourceCode);
    }
    else {
        // Interactive mode
        init_interpreter();
        getchar();
    }

    return 0;
}








