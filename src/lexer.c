/***********************************************************
* File: lexer.c
* This file contains the lexer implementation for the interpreter.
* The lexer will tokenize the source code.
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "lexer.h"





/***********************************************************
* Struct: OperatorEntry
* Description: This struct is used to map operators to their
* respective token types.
************************************************************/
OperatorEntry OPERATORS[] = {
    { "==",  TOKEN_EQUALS_EQUALS },
    { "!=",  TOKEN_NOT_EQUALS },
    { "<=",  TOKEN_LESSTHANOREQUALS },
    { ">=",  TOKEN_GREATERTHANOREQUALS },
    { "<<",  TOKEN_LEFTSHIFT },
    { ">>",  TOKEN_RIGHTSHIFT },
    { "->",  TOKEN_POINTER },
    { "+=",  TOKEN_PLUS_EQUALS },
    { "-=",  TOKEN_MINUS_EQUALS },
    { "*=",  TOKEN_MULT_EQUALS },
    { "/=",  TOKEN_DIV_EQUALS },
    { "%=",  TOKEN_MOD_EQUALS },
    { "++",  TOKEN_INCREMENT },
    {"--",   TOKEN_DECREMENT},
    { "+",   TOKEN_PLUS },
    { "-",   TOKEN_MINUS },
    { "*",   TOKEN_MULTIPLY },
    { "/",   TOKEN_DIVIDE },
    { "=",   TOKEN_EQUALS },
    { "<",   TOKEN_LESSTHAN },
    { ">",   TOKEN_GREATERTHAN },
    { "!",   TOKEN_NOT },
    { "&&",  TOKEN_AND },
    { "||",  TOKEN_OR },
    { "^",   TOKEN_XOR },
    { "~",   TOKEN_COMPLEMENT },
    { "{",   TOKEN_BEGINBLOCK },
    { "}",   TOKEN_ENDBLOCK },
    { "(",   TOKEN_BEGINPARAMS },
    { ")",   TOKEN_ENDPARAMS },
    { ";",   TOKEN_END },
    { ",",   TOKEN_SEPARATION },
    { "%",   TOKEN_MOD },
    { ":",  TOKEN_COLON },
    { "[" , TOKEN_BEGININDEX },
    { "]", TOKEN_ENDINDEX }
};






/***********************************************************
* Struct: KeywordEntry
* Description: This struct is used to map keywords to their
* respective token types.
* PS: keywords as operators are not supported yet.
* ************************************************************/
KeywordEntry KEYWORDS[] = {
    // Operators as Keywords
    { "plus",                TOKEN_PLUS },
    { "minus",               TOKEN_MINUS },
    { "multiply",            TOKEN_MULTIPLY },
    { "divide",              TOKEN_DIVIDE },
    { "equals",              TOKEN_EQUALS },
    { "equalsEquals",        TOKEN_EQUALS_EQUALS },
    { "notEquals",           TOKEN_NOT_EQUALS },
    { "lessThan",            TOKEN_LESSTHAN },
    { "lessThanOrEquals",    TOKEN_LESSTHANOREQUALS },
    { "greaterThan",         TOKEN_GREATERTHAN },
    { "greaterThanOrEquals", TOKEN_GREATERTHANOREQUALS },
    { "and",                 TOKEN_AND },
    { "or",                  TOKEN_OR },
    { "xor",                 TOKEN_XOR },
    { "complement",          TOKEN_COMPLEMENT },
    { "leftShift",           TOKEN_LEFTSHIFT },
    { "rightShift",          TOKEN_RIGHTSHIFT },
    { "pointer",             TOKEN_POINTER },
    { "plusEquals",          TOKEN_PLUS_EQUALS },
    { "minusEquals",         TOKEN_MINUS_EQUALS },
    { "multiplyEquals",      TOKEN_MULT_EQUALS },
    { "divideEquals",        TOKEN_DIV_EQUALS },
    { "modEquals",           TOKEN_MOD_EQUALS },

    // Block Tokens
    { "doing",       TOKEN_BEGINBLOCK },
    { "finish",      TOKEN_ENDBLOCK },
    { "open",        TOKEN_BEGINPARAMS },
    { "close",       TOKEN_ENDPARAMS },
    { "to",          TOKEN_TO },
    { "end",         TOKEN_END },

    // Variable Types
    { "make",        TOKEN_VAR },
    { "list",        TOKEN_ARRAY },
    { "const",       TOKEN_CONST },
    { "true",        TOKEN_TRUE },
    { "false",       TOKEN_FALSE },
    {"function", TOKEN_FUNCTION},

    // Also synonyms for parentheses if desired:
    { "beginParams", TOKEN_BEGINPARAMS },
    { "endParams",   TOKEN_ENDPARAMS },

    // Keywords
    { "if",        TOKEN_IF },
    { "else",      TOKEN_ELSE },
    { "while",     TOKEN_WHILE },
    { "for",       TOKEN_FOR },
    { "return",    TOKEN_RETURN },
    { "none",      TOKEN_VOID },
    { "NULL",      TOKEN_NULL },
    { "stop",       TOKEN_BREAK },
    { "continue", TOKEN_CONTINUE },
    { "switch" ,  TOKEN_SWITCH },
    { "when",    TOKEN_WHEN },
    { "default", TOKEN_DEFAULT }
};

static const size_t KEYWORDS_COUNT = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);    // Number of keywords
static const size_t OPERATORS_COUNT = sizeof(OPERATORS) / sizeof(OPERATORS[0]); // Number of operators






/***********************************************************
* Function: strndump
* Description: This function is used to copy a string of a
* given length.
* Parameters: const char* str, size_t n
* Return: char*
* ***********************************************************/
char* strndump(const char* str, size_t n) {
    char* copy = (char*)malloc(n + 1);
    if (!copy) {
        fprintf(stderr, "Memory allocation failed in strndump\n");
        exit(EXIT_FAILURE);
    }

        #ifdef _WIN32
            strncpy_s(copy, n + 1, str, n);
        #else
            strncpy(copy, str, n);
            copy[n] = '\0';
        #endif

    copy[n] = '\0';
    return copy;
}





/***********************************************************
* Function: lookup_operator
* Description: This function is used to lookup an operator
* in the OPERATORS array.
* Parameters: const char* str
* Return: TokenKind
* ***********************************************************/
TokenKind lookup_operator(const char* str) {
    for (size_t i = 0; i < OPERATORS_COUNT; i++) {
        if (strcmp(str, OPERATORS[i].op) == 0) {
            return OPERATORS[i].type;
        }
    }
    return TOKEN_EOF; // Return TOKEN_EOF if no match is found
}





/***********************************************************
* Function: lookup_keyword
* Description: This function is used to lookup a keyword
* in the KEYWORDS array.
* Parameters: const char* str
* Return: TokenKind
* ***********************************************************/
TokenKind lookup_keyword(const char* str) {
    for (size_t i = 0; i < KEYWORDS_COUNT; i++) {
        if (strcmp(str, KEYWORDS[i].keyword) == 0) {
            return KEYWORDS[i].type;
        }
    }
    return TOKEN_IDENTIFIER; // Default to TOKEN_IDENTIFIER if no match is found
}





/***********************************************************
* Function: init_token_array
* Description: This function is used to initialize a TokenArray.
* Parameters: TokenArray* arr
* Return: void
* ***********************************************************/
void init_token_array(TokenArray* arr) {
    arr->size = 0;
    arr->capacity = 16;
    arr->data = (Token*)malloc(sizeof(Token) * arr->capacity);
    if (!arr->data) {
        fprintf(stderr, "Memory allocation failed in init_token_array\n");
        exit(EXIT_FAILURE);
    }
    // Initialize to defaults
    for (size_t i = 0; i < arr->capacity; i++) {
        arr->data[i].value = NULL;
        arr->data[i].type = TOKEN_EOF;
        arr->data[i].column = 0;
        arr->data[i].line = 0;
    }
}






/***********************************************************
* Function: free_token_array
* Description: This function is used to free the memory used
* by a TokenArray.
* Parameters: TokenArray* arr
* Return: void
* ***********************************************************/
void free_token_array(TokenArray* arr) {
    for (size_t i = 0; i < arr->size; i++) {
        if (arr->data[i].value) {
            free(arr->data[i].value);
        }
    }
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}





/***********************************************************
* Function: push_token
* Description: This function is used to add a token to a TokenArray.
* Parameters: TokenArray* arr, Token t
* Return: void
* ***********************************************************/
void push_token(TokenArray* arr, Token t) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        Token* temp = (Token*)realloc(arr->data, sizeof(Token) * arr->capacity);
        if (!temp) {
            fprintf(stderr, "Memory allocation failed in push_token\n");
            free_token_array(arr);
            exit(EXIT_FAILURE);
        }
        arr->data = temp;
    }
    // Only push if token has a valid value pointer
    if (t.value) {
        arr->data[arr->size++] = t;
    }
    else {
        fprintf(stderr, "Invalid token value\n");
    }
}





/***********************************************************
* Function: make_token
* Description: This function is used to create a token with a
* given value and type.
* Parameters: const char* value, TokenKind type
* Return: Token
* ***********************************************************/
Token make_token(const char* value, TokenKind type) {
    Token t;
    t.type = type;
    t.column = 0;
    t.line = 0;
    if (value) {
        size_t len = strlen(value);
        t.value = (char*)malloc(len + 1);
        if (!t.value) {
            fprintf(stderr, "Memory allocation failed for token value\n");
            exit(EXIT_FAILURE);
        }
            #ifdef _WIN32
                strcpy_s(t.value, len + 1, value);
            #else
                strncpy(t.value, value, len);
                t.value[len] = '\0';
            #endif

    }
    else {
        t.value = NULL;
    }
    return t;
}






/***********************************************************
* Function: is_hex_digit
* Description: This function is used to check if a character
* can be part of a hex digit (0-9, a-f, A-F).
* Parameters: char c
* Return: int
* ***********************************************************/
int is_hex_digit(char c) {
    return (isdigit((unsigned char)c) ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F'));
}





/***********************************************************
* Function: is_bin_digit
* Description: This function is used to check if a character
* can be part of a binary digit (0 or 1).
* Parameters: char c
* Return: int
* ***********************************************************/
int is_bin_digit(char c) {
    return (c == '0' || c == '1');
}




/***********************************************************
* Function: lex_number_or_float
* Description: This function is used to tokenize a number or
* a float.
* Parameters: const char* source, size_t* pos, size_t length,
* size_t* lineNo, size_t* colNo
* Return: Token
* ***********************************************************/
Token lex_number_or_float(const char* source, size_t* pos, size_t length,
    size_t* lineNo, size_t* colNo)
{
    size_t startPos = *pos;
    size_t startCol = *colNo;
    size_t startLine = *lineNo;

    // Check for 0x or 0b (hex/binary)
    int isHex = 0;
    int isBin = 0;
    int isFloat = 0;

    // If first char is '0' and second char is 'x' => hex
    // If first char is '0' and second char is 'b' => bin
    // BUT only if there's no '.' after '0' => e.g. '0.5' is float, not hex
    if (source[*pos] == '0') {
        if ((*pos + 1 < length) && (source[*pos + 1] == 'x' || source[*pos + 1] == 'X')) {
            isHex = 1;
        }
        else if ((*pos + 1 < length) && (source[*pos + 1] == 'b' || source[*pos + 1] == 'B')) {
            isBin = 1;
        }
    }

    if (isHex) {
        // skip '0x'
        (*pos) += 2;
        (*colNo) += 2;

        // read hex digits
        while ((*pos) < length && is_hex_digit(source[*pos])) {
            (*pos)++;
            (*colNo)++;
        }
    }
    else if (isBin) {
        // skip '0b'
        (*pos) += 2;
        (*colNo) += 2;

        while ((*pos) < length && is_bin_digit(source[*pos])) {
            (*pos)++;
            (*colNo)++;
        }
    }
    else {
        // decimal or float
        // read digits
        while ((*pos) < length && isdigit((unsigned char)source[*pos])) {
            (*pos)++;
            (*colNo)++;
        }

        // if next char is '.' => treat as float
        if ((*pos) < length && source[*pos] == '.') {
            // but only if next after '.' is a digit => e.g. '3.14'
            if ((*pos + 1) < length && isdigit((unsigned char)source[*pos + 1])) {
                isFloat = 1;
                // consume '.'
                (*pos)++;
                (*colNo)++;
                // read fractional digits
                while ((*pos) < length && isdigit((unsigned char)source[*pos])) {
                    (*pos)++;
                    (*colNo)++;
                }
            }
        }

        // optional exponent? e.g. 3.14e+10 or 3e-2 thought of doing it but gave up :p
        if (isFloat) {
            // check if there's an 'e' or 'E'
            if ((*pos) < length && (source[*pos] == 'e' || source[*pos] == 'E')) {
                // consume 'e'
                (*pos)++;
                (*colNo)++;
                // optional +/-
                if ((*pos) < length && (source[*pos] == '+' || source[*pos] == '-')) {
                    (*pos)++;
                    (*colNo)++;
                }
                // read exponent digits
                int exponentDigits = 0;
                while ((*pos) < length && isdigit((unsigned char)source[*pos])) {
                    (*pos)++;
                    (*colNo)++;
                    exponentDigits++;
                }
                // if exponentDigits==0 => malformed exponent => handle error if you want, i will come back here later
            }
        }
    }

    size_t tokenLen = (*pos) - startPos;
    char* numStr = strndump(&source[startPos], tokenLen);

    TokenKind tkType = TOKEN_NUMBER;
    if (isHex) {
        tkType = TOKEN_HEX;
    }
    else if (isBin) {
        tkType = TOKEN_BINARY;
    }
    else if (isFloat) {
        tkType = TOKEN_FLOAT;
    }

    Token t = make_token(numStr, tkType);
    free(numStr);

    t.line = startLine;
    t.column = startCol;
    return t;
}





/***********************************************************
* Function: lex_string
* Description: This function is used to tokenize a string.
* Parameters: const char* source, size_t* pos, size_t length,
* size_t* lineNo, size_t* colNo
* Return: Token
* ***********************************************************/
Token lex_string(const char* source, size_t* pos, size_t length,
    size_t* lineNo, size_t* colNo)
{
    size_t startPos = *pos;    // position is at the opening quote
    size_t startCol = *colNo;
    size_t startLine = *lineNo;

    // skip the initial quote
    (*pos)++;
    (*colNo)++;

    size_t strStart = *pos;

    while ((*pos) < length && source[*pos] != '\"') {
        if (source[*pos] == '\n') {
            // allow multiline? or error out? not sure yet but in the end sounds cool I guess?
            // We'll allow but increment line
            (*lineNo)++;
            (*colNo) = 1;
        }
        else {
            (*colNo)++;
        }
        (*pos)++;
    }

    if ((*pos) >= length) {
        // Reached end of file without closing quote
        fprintf(stderr, "Unterminated string at line %zu, col %zu\n", *lineNo, *colNo);
        // We'll create a token anyway for debugging
    }

    // Now source[*pos] is either end or the closing quote
    size_t stringLen = (*pos) - strStart;
    char* strText = strndump(&source[strStart], stringLen);

    // if we haven't hit the end, skip the closing quote
    if ((*pos) < length && source[*pos] == '\"') {
        (*pos)++;
        (*colNo)++;
    }

    Token t = make_token(strText, TOKEN_STRING);
    free(strText);

    t.line = startLine;
    t.column = startCol;
    return t;
}





/***********************************************************
* Function: lex_char
* Description: This function is used to tokenize a character.
* Parameters: const char* source, size_t* pos, size_t length,
* size_t* lineNo, size_t* colNo
* Return: Token
* ***********************************************************/
Token lex_char(const char* source, size_t* pos, size_t length,
    size_t* lineNo, size_t* colNo)
{
    size_t startPos = *pos;    // position is at the opening quote
    size_t startCol = *colNo;
    size_t startLine = *lineNo;

    // skip the initial quote
    (*pos)++;
    (*colNo)++;

    size_t charStart = *pos;

    // We'll read until the next `'` or end-of-file
    while ((*pos) < length && source[*pos] != '\'') {
        if (source[*pos] == '\n') {
            // Char should only be one char, but once we have a runtime lets mess more with strings
            fprintf(stderr, "Unterminated character literal at line %zu, col %zu\n", *lineNo, *colNo);
            break;
        }
        (*pos)++;
        (*colNo)++;
    }

    size_t charLen = 0;
    if ((*pos) < length) {
        // The substring is from [charStart..(*pos)-1]
        charLen = (*pos) - charStart;
    }
    else {
        // End of file
        fprintf(stderr, "Unterminated character literal at line %zu, col %zu\n", *lineNo, *colNo);
    }

    char* charText = strndump(&source[charStart], charLen);

    // Skip the closing quote if present
    if ((*pos) < length && source[*pos] == '\'') {
        (*pos)++;
        (*colNo)++;
    }

    Token t = make_token(charText, TOKEN_CHAR);
    free(charText);

    t.line = startLine;
    t.column = startCol;
    return t;
}






/***********************************************************
* Function: tokenize
* Description: This function is used to tokenize the source code.
* Parameters: const char* source
* Return: TokenArray
* ***********************************************************/
TokenArray tokenize(const char* source) {
    TokenArray tokens;
    init_token_array(&tokens);

    size_t position = 0;
    size_t length = strlen(source);

    size_t lineNo = 1;
    size_t colNo = 1;

    while (position < length) {
        char current = source[position];

        // 1) Handle newlines
        if (current == '\n') {
            lineNo++;
            colNo = 1;
            position++;
            continue;
        }

        // 2) Handle other whitespace
        if (isspace((unsigned char)current)) {
            position++;
            colNo++;
            continue;
        }

        // 3) Single-line comment (# or //)
        if (current == '#' ||
            (current == '/' && (position + 1 < length) && source[position + 1] == '/'))
        {
            // skip until newline
            while (position < length && source[position] != '\n') {
                position++;
                colNo++;
            }
            continue;
        }

        // 4) Try to match an operator
        {
            int operator_matched = 0;
            for (size_t i = 0; i < OPERATORS_COUNT; i++) {
                size_t op_length = strlen(OPERATORS[i].op);
                if (position + op_length <= length &&
                    strncmp(&source[position], OPERATORS[i].op, op_length) == 0)
                {
                    Token t = make_token(OPERATORS[i].op, OPERATORS[i].type);
                    t.line = lineNo;
                    t.column = colNo;

                    push_token(&tokens, t);

                    position += op_length;
                    colNo += op_length;
                    operator_matched = 1;
                    break;
                }
            }
            if (operator_matched) {
                continue;
            }
        }

        // 5) String literal?
        if (current == '\"') {
            Token sToken = lex_string(source, &position, length, &lineNo, &colNo);
            push_token(&tokens, sToken);
            continue;
        }

        // 6) Number literal? (decimal, hex, binary)
        //    If current is digit or current == '0' (leading to 0x or 0b)
        if (isdigit((unsigned char)current)) {
            Token numToken = lex_number_or_float(source, &position, length, &lineNo, &colNo);
            push_token(&tokens, numToken);
            continue;
        }

        // 7) Identifier or Keyword? (must start with alpha or '_')
        //    This check goes AFTER number check, so we don't mix up 0x with identifier logic
        if (isalpha((unsigned char)current) || current == '_') {
            size_t start = position;
            size_t startCol = colNo;
            while (position < length &&
                (isalnum((unsigned char)source[position]) ||
                    source[position] == '_'))
            {
                position++;
                colNo++;
            }
            size_t identLen = position - start;
            char* identifier = strndump(&source[start], identLen);

            TokenKind type = lookup_keyword(identifier);
            Token t = make_token(identifier, type);
            t.line = lineNo;
            t.column = startCol;

            push_token(&tokens, t);
            free(identifier);
            continue;
        }

        // 8) If we reach here, it's an unexpected character
        fprintf(stderr, "Unexpected character '%c' at line %zu, col %zu\n",
            current, lineNo, colNo);
        position++;
        colNo++;
    }

    // Add final EOF token for the program
    {
        Token eofToken = make_token("", TOKEN_EOF);
        eofToken.line = lineNo;
        eofToken.column = colNo;
        push_token(&tokens, eofToken);
    }

    return tokens;
}
