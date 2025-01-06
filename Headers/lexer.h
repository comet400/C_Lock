/***********************************************************
* File: lexer.h
* This file contains the lexer for the interpreter.
* The lexer is responsible for tokenizing the source code.
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/

#pragma once

#ifndef LEXER_H
#define LEXER_H

typedef enum {
    // Operators
    TOKEN_PLUS,             // +
    TOKEN_MINUS,            // -
    TOKEN_MULTIPLY,         // *
    TOKEN_DIVIDE,           // /
    TOKEN_EQUALS,           // =
    TOKEN_LESSTHAN,         // <
    TOKEN_LESSTHANOREQUALS, // <=
    TOKEN_EQUALS_EQUALS,    // ==
    TOKEN_GREATERTHAN,      // >
    TOKEN_GREATERTHANOREQUALS, // >=
    TOKEN_NOT_EQUALS,       // !=
    TOKEN_AND,              // (Currently used for '&&' => logical AND, or bitwise AND if you prefer)
    TOKEN_OR,               // (Currently used for '||' => logical OR, or bitwise OR if you prefer)
    TOKEN_XOR,              // ^
    TOKEN_COMPLEMENT,       // ~
    TOKEN_LEFTSHIFT,        // <<
    TOKEN_RIGHTSHIFT,       // >>
    TOKEN_POINTER,          // ->
    TOKEN_PLUS_EQUALS,      // +=
    TOKEN_MINUS_EQUALS,     // -=
    TOKEN_MULT_EQUALS,      // *=
    TOKEN_DIV_EQUALS,       // /=
    TOKEN_MOD_EQUALS,       // %=
    TOKEN_NOT,              // !
    TOKEN_MOD,              // %
    TOKEN_INCREMENT,        // ++
    TOKEN_DECREMENT,        // --
    TOKEN_SEPARATION,        // ,
    TOKEN_COLON,            // :

    // Block Tokens
    TOKEN_BEGINBLOCK,       // {
    TOKEN_ENDBLOCK,         // }

    // Function Parameter Tokens
    TOKEN_BEGINPARAMS,      // (
    TOKEN_ENDPARAMS,        // )

    TOKEN_BEGININDEX,       // [
    TOKEN_ENDINDEX,         // ]

    // Literals
    TOKEN_NUMBER,           // Numeric literal
    TOKEN_BINARY,           // Binary literal: 0b1010
    TOKEN_HEX,              // Hexadecimal literal: 0x1A3F
    TOKEN_STRING,           // String literal: "..."
    TOKEN_FLOAT,            // Floating-point literal
    TOKEN_CHAR,             // Character literal: 'a'


    // Keywords
    TOKEN_IF,               // if
    TOKEN_ELSE,             // else
    TOKEN_WHILE,            // while
    TOKEN_FOR,              // for
    TOKEN_RETURN,           // return
    TOKEN_VOID,             // void
    TOKEN_VAR,              // variable
    TOKEN_ARRAY,            // array
    TOKEN_CONST,            // const (noChange)
    TOKEN_END,              // end
    TOKEN_TRUE,             // true
    TOKEN_FALSE,            // false
    TOKEN_KEY_LESS,         // lessThan (renamed from TOKEN_LESSTHAN)
    TOKEN_KEY_LESSEQUAL,    // lessThanOrEquals (renamed from TOKEN_LESSTHANOREQUALS)
    TOKEN_KEY_GREATER,      // greaterThan (renamed from TOKEN_GREATERTHAN)
    TOKEN_KEY_GREATEREQUAL, // greaterThanOrEquals (renamed from TOKEN_GREATERTHANOREQUALS)
    TOKEN_KEY_EQUAL,        // equal (keyword)
    TOKEN_KEY_NOTEQUAL,     // notEqual (keyword)
    TOKEN_AND_KEYWORD,      // and (logical and)
    TOKEN_OR_KEYWORD,       // or (logical or)
    TOKEN_TO,               // to
    TOKEN_FUNCTION,         // function
    TOKEN_ADD,              // add
    TOKEN_SUBTRACT,         // subtract
    TOKEN_MULTIPLY_KEYWORD, // multiply
    TOKEN_DIVIDE_KEYWORD,   // divide
    TOKEN_IFNOT,            // ifNot
    TOKEN_WHILENOT,         // whileNot
    TOKEN_NULL,             // null
    TOKEN_BREAK,            // break
    TOKEN_CONTINUE,         // continue
    TOKEN_SWITCH,           // switch
    TOKEN_WHEN,             // when
    TOKEN_DEFAULT,          // default

    // General
    TOKEN_IDENTIFIER,       // Variable/function name
    TOKEN_EOF               // End of file
} TokenKind;

typedef struct {
    const char* op;   // The operator string (e.g., "==", "->")
    TokenKind   type; // Corresponding TokenType
} OperatorEntry;

typedef struct {
    const char* keyword; // The keyword string (e.g., "if", "else")
    TokenKind   type;    // Corresponding TokenType
} KeywordEntry;

typedef struct {
    char* value; // Token value as a string
    TokenKind type; // Token type
    size_t column; // Column number where the token starts
    size_t line; // Line number where the token starts
} Token;

typedef struct {
    const char* source; // Source code to tokenize
    size_t      index;
    size_t      line;
    size_t      column;
} Lexer;

typedef struct {
    Token* data;     // Pointer to the first element
    size_t  size;   // Number of elements
    size_t  capacity; // Number of elements
} TokenArray;

TokenKind lookup_operator(const char* str); // Lookup an operator

TokenKind lookup_keyword(const char* str); // Lookup a keyword

void init_token_array(TokenArray* arr); // Initialize the array

void free_token_array(TokenArray* arr); // Free the memory used by the array

void push_token(TokenArray* arr, Token t); // Add a token to the array

Token make_token(const char* value, TokenKind type); // Create a token with a value and type

TokenArray tokenize(const char* source); // Tokenize the source code

char* strndump(const char* str, size_t n); // Duplicate n characters of a string because strndup was not working

Token lex_char(const char* source, size_t* pos, size_t length,
    size_t* lineNo, size_t* colNo); // Lex a character literal

Token lex_string(const char* source, size_t* pos, size_t length,
    size_t* lineNo, size_t* colNo); // Lex a string literal

Token lex_number_or_float(const char* source, size_t* pos, size_t length,
    size_t* lineNo, size_t* colNo); // Lex a number or a float

#endif // LEXER_H
