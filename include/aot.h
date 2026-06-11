#pragma once

#ifndef AOT_H
#define AOT_H

#include <stdbool.h>
#include <stddef.h>
#include "ast.h"

bool aot_compile_ast(const ASTNode* root, const char* output_binary, char* error_buffer, size_t error_capacity);

#endif // AOT_H
