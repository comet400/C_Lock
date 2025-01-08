#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"


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

int main()
{
	const char* source = "make lukas = 10; if (lukas == 10) { return 10;} else { return 40;} make bob = 0b1010;";
	TokenArray tokens = tokenize(source);
	print_tokens(&tokens);
	free_token_array(&tokens);
	return 0;
}

