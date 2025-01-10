#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
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
    system("cls");
    // 1) Prompt user for input
    //change print color to green
    system("color 0A");

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
        strcpy_s(sourceCode + currentLength, bufferSize - currentLength, line);
        currentLength += lineLen;
        fullCode = sourceCode;
        system("cls");
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

    // 4) Interpret (execute) the AST
    interpret(root);


    // 5) Clean up: free AST, tokens, etc.
    free_ast_node(root);
    free_token_array(&tokens);
    free(sourceCode);

    //go back to white
    printf("\033[0;37m");
}


int main(int argc, char* argv[]) {
    if (argc == 2) {
        // File mode
        const char* filename = argv[1];
        FILE* file = fopen(filename, "r");
        if (!file) {
            perror("Error opening file");
            return 1;
        }

        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* sourceCode = (char*)malloc(length + 1);
        if (!sourceCode) {
            fprintf(stderr, "Memory allocation failed.\n");
            fclose(file);
            return 1;
        }

        fread(sourceCode, 1, length + 1, file);
        sourceCode[length] = '\0';
        fclose(file);

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
		printf("\033[0;37m");
    }
    else {
        // Interactive mode
        init_interpreter();
        getchar();
    }

    return 0;
}








