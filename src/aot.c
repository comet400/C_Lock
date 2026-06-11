#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "aot.h"

typedef struct {
    FILE* out;
    int indent;
    int temp_counter;
    int label_counter;
    bool in_function;
} AOTCtx;

static void set_error(char* err, size_t cap, const char* fmt, ...) {
    if (!err || cap == 0) return;
    va_list args;
    va_start(args, fmt);
    vsnprintf(err, cap, fmt, args);
    va_end(args);
}

static void emit_indent(AOTCtx* ctx) {
    for (int i = 0; i < ctx->indent; i++) fputs("    ", ctx->out);
}

static void emit_line(AOTCtx* ctx, const char* fmt, ...) {
    emit_indent(ctx);
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);
    fputc('\n', ctx->out);
}

static char* next_temp(AOTCtx* ctx) {
    char buf[64];
    snprintf(buf, sizeof(buf), "__t%d", ctx->temp_counter++);
    char* out = (char*)malloc(strlen(buf) + 1);
    strcpy(out, buf);
    return out;
}

static char* next_label(AOTCtx* ctx, const char* base) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s_%d", base, ctx->label_counter++);
    char* out = (char*)malloc(strlen(buf) + 1);
    strcpy(out, buf);
    return out;
}

static bool emit_expr(AOTCtx* ctx, const ASTNode* node, const char* out_name, char* err, size_t errcap);
static bool emit_stmt(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap);

static void emit_escaped_c_string(FILE* out, const char* s) {
    fputc('"', out);
    if (s) {
        for (const char* p = s; *p; ++p) {
            if (*p == '\\' || *p == '"') {
                fputc('\\', out);
                fputc(*p, out);
            } else if (*p == '\n') {
                fputs("\\n", out);
            } else if (*p == '\t') {
                fputs("\\t", out);
            } else {
                fputc(*p, out);
            }
        }
    }
    fputc('"', out);
}

static bool emit_literal(AOTCtx* ctx, const ASTNode* node, const char* out_name) {
    switch (node->value_kind) {
        case VALUE_INT:
            emit_line(ctx, "RuntimeValue %s = make_int_value(%ld);", out_name, node->value.int_val);
            return true;
        case VALUE_FLOAT:
            emit_line(ctx, "RuntimeValue %s = make_float_value(%g);", out_name, node->value.float_val);
            return true;
        case VALUE_BOOL:
            emit_line(ctx, "RuntimeValue %s = make_bool_value(%s);", out_name, node->value.bool_val ? "true" : "false");
            return true;
        case VALUE_STRING:
            emit_indent(ctx);
            fprintf(ctx->out, "RuntimeValue %s = make_string_value(", out_name);
            emit_escaped_c_string(ctx->out, node->value.str_val ? node->value.str_val : "");
            fprintf(ctx->out, ");\n");
            return true;
        default:
            emit_line(ctx, "RuntimeValue %s = make_null_value();", out_name);
            return true;
    }
}

static bool emit_identifier(AOTCtx* ctx, const ASTNode* node, const char* out_name) {
    emit_line(ctx, "RuntimeValue %s = env_get_var(env, \"%s\");", out_name, node->operator_ ? node->operator_ : "");
    emit_line(ctx, "if (%s.type == RUNTIME_VALUE_NULL) %s = env_get_func(env, \"%s\");", out_name, out_name, node->operator_ ? node->operator_ : "");
    return true;
}

static bool emit_call(AOTCtx* ctx, const ASTNode* node, const char* out_name, char* err, size_t errcap) {
    if (node->child_count < 1) {
        set_error(err, errcap, "AOT: invalid function call");
        return false;
    }

    const ASTNode* fn = node->children[0];
    if (!fn || fn->type != AST_IDENTIFIER || !fn->operator_) {
        set_error(err, errcap, "AOT: only identifier calls supported");
        return false;
    }

    size_t argc = node->child_count - 1;
    char* fn_tmp = next_temp(ctx);
    emit_line(ctx, "RuntimeValue %s = make_null_value();", out_name);
    emit_line(ctx, "{ /* call %s */", fn->operator_);
    ctx->indent++;
    emit_line(ctx, "RuntimeValue %s = env_get_func(env, \"%s\");", fn_tmp, fn->operator_);
    emit_line(ctx, "if (%s.type != RUNTIME_VALUE_BUILTIN) { fprintf(stderr, \"AOT runtime: function not callable: %s\\n\"); %s = make_null_value(); } else {", fn_tmp, fn->operator_, out_name);
    ctx->indent++;
    emit_line(ctx, "RuntimeValue __args[%zu];", argc > 0 ? argc : 1);
    for (size_t i = 0; i < argc; i++) {
        char* tmp = next_temp(ctx);
        if (!emit_expr(ctx, node->children[i + 1], tmp, err, errcap)) {
            free(tmp);
            free(fn_tmp);
            ctx->indent -= 2;
            emit_line(ctx, "}");
            emit_line(ctx, "}");
            return false;
        }
        emit_line(ctx, "__args[%zu] = %s;", i, tmp);
        free(tmp);
    }
    emit_line(ctx, "%s = %s.builtin_val.fn(__args, %zu);", out_name, fn_tmp, argc);
    ctx->indent--;
    emit_line(ctx, "}");
    ctx->indent--;
    emit_line(ctx, "} /* end call %s */", fn->operator_);
    free(fn_tmp);
    return true;
}

static bool emit_binary(AOTCtx* ctx, const ASTNode* node, const char* out_name, char* err, size_t errcap) {
    if (node->child_count < 2 || !node->operator_) {
        set_error(err, errcap, "AOT: invalid binary expression");
        return false;
    }

    char* left = next_temp(ctx);
    char* right = next_temp(ctx);
    if (!emit_expr(ctx, node->children[0], left, err, errcap) ||
        !emit_expr(ctx, node->children[1], right, err, errcap)) {
        free(left);
        free(right);
        return false;
    }

    emit_line(ctx, "RuntimeValue %s = make_null_value();", out_name);

    if (strcmp(node->operator_, "+") == 0 || strcmp(node->operator_, "-") == 0 ||
        strcmp(node->operator_, "*") == 0 || strcmp(node->operator_, "/") == 0 ||
        strcmp(node->operator_, "%") == 0) {
        emit_line(ctx, "if (%s.type == RUNTIME_VALUE_FLOAT || %s.type == RUNTIME_VALUE_FLOAT) {", left, right);
        ctx->indent++;
        emit_line(ctx, "double __a = (%s.type == RUNTIME_VALUE_FLOAT) ? %s.float_val : (double)%s.int_val;", left, left, left);
        emit_line(ctx, "double __b = (%s.type == RUNTIME_VALUE_FLOAT) ? %s.float_val : (double)%s.int_val;", right, right, right);
        if (strcmp(node->operator_, "+") == 0) emit_line(ctx, "%s = make_float_value(__a + __b);", out_name);
        if (strcmp(node->operator_, "-") == 0) emit_line(ctx, "%s = make_float_value(__a - __b);", out_name);
        if (strcmp(node->operator_, "*") == 0) emit_line(ctx, "%s = make_float_value(__a * __b);", out_name);
        if (strcmp(node->operator_, "/") == 0) emit_line(ctx, "%s = make_float_value(__a / __b);", out_name);
        if (strcmp(node->operator_, "%") == 0) emit_line(ctx, "%s = make_int_value((long)__a %% (long)__b);", out_name);
        ctx->indent--;
        emit_line(ctx, "} else {");
        ctx->indent++;
        if (strcmp(node->operator_, "+") == 0) emit_line(ctx, "%s = make_int_value(%s.int_val + %s.int_val);", out_name, left, right);
        if (strcmp(node->operator_, "-") == 0) emit_line(ctx, "%s = make_int_value(%s.int_val - %s.int_val);", out_name, left, right);
        if (strcmp(node->operator_, "*") == 0) emit_line(ctx, "%s = make_int_value(%s.int_val * %s.int_val);", out_name, left, right);
        if (strcmp(node->operator_, "/") == 0) emit_line(ctx, "%s = make_int_value(%s.int_val / %s.int_val);", out_name, left, right);
        if (strcmp(node->operator_, "%") == 0) emit_line(ctx, "%s = make_int_value(%s.int_val %% %s.int_val);", out_name, left, right);
        ctx->indent--;
        emit_line(ctx, "}");
    } else if (strcmp(node->operator_, "==") == 0) {
        emit_line(ctx, "%s = make_bool_value(%s.int_val == %s.int_val);", out_name, left, right);
    } else if (strcmp(node->operator_, "!=") == 0) {
        emit_line(ctx, "%s = make_bool_value(%s.int_val != %s.int_val);", out_name, left, right);
    } else if (strcmp(node->operator_, "<") == 0) {
        emit_line(ctx, "%s = make_bool_value(%s.int_val < %s.int_val);", out_name, left, right);
    } else if (strcmp(node->operator_, ">") == 0) {
        emit_line(ctx, "%s = make_bool_value(%s.int_val > %s.int_val);", out_name, left, right);
    } else if (strcmp(node->operator_, "<=") == 0) {
        emit_line(ctx, "%s = make_bool_value(%s.int_val <= %s.int_val);", out_name, left, right);
    } else if (strcmp(node->operator_, ">=") == 0) {
        emit_line(ctx, "%s = make_bool_value(%s.int_val >= %s.int_val);", out_name, left, right);
    } else {
        emit_line(ctx, "%s = make_null_value();", out_name);
    }

    free(left);
    free(right);
    return true;
}

static bool emit_unary(AOTCtx* ctx, const ASTNode* node, const char* out_name, char* err, size_t errcap) {
    if (node->child_count < 1 || !node->operator_) {
        set_error(err, errcap, "AOT: invalid unary expression");
        return false;
    }

    char* inner = next_temp(ctx);
    if (!emit_expr(ctx, node->children[0], inner, err, errcap)) {
        free(inner);
        return false;
    }

    if (strcmp(node->operator_, "-") == 0) {
        emit_line(ctx, "RuntimeValue %s = (%s.type == RUNTIME_VALUE_FLOAT) ? make_float_value(-%s.float_val) : make_int_value(-%s.int_val);", out_name, inner, inner, inner);
    } else if (strcmp(node->operator_, "!") == 0) {
        emit_line(ctx, "RuntimeValue %s = make_bool_value(!((%s.type == RUNTIME_VALUE_BOOL && %s.bool_val) || (%s.type == RUNTIME_VALUE_INT && %s.int_val != 0)));", out_name, inner, inner, inner, inner);
    } else {
        emit_line(ctx, "RuntimeValue %s = make_null_value();", out_name);
    }

    free(inner);
    return true;
}

static bool emit_array_literal(AOTCtx* ctx, const ASTNode* node, const char* out_name, char* err, size_t errcap) {
    emit_line(ctx, "RuntimeValue %s = make_null_value();", out_name);
    set_error(err, errcap, "AOT: array literal not yet supported in AOT C backend");
    return false;
}

static bool emit_expr(AOTCtx* ctx, const ASTNode* node, const char* out_name, char* err, size_t errcap) {
    if (!node) {
        emit_line(ctx, "RuntimeValue %s = make_null_value();", out_name);
        return true;
    }

    switch (node->type) {
        case AST_LITERAL:
            return emit_literal(ctx, node, out_name);
        case AST_IDENTIFIER:
            return emit_identifier(ctx, node, out_name);
        case AST_BINARY_EXPR:
            return emit_binary(ctx, node, out_name, err, errcap);
        case AST_UNARY_EXPR:
            return emit_unary(ctx, node, out_name, err, errcap);
        case AST_FUNCTION_CALL:
            return emit_call(ctx, node, out_name, err, errcap);
        case AST_ARRAY_ACCESS: {
            if (node->child_count != 2) {
                set_error(err, errcap, "AOT: invalid array access");
                return false;
            }
            char* arr = next_temp(ctx);
            char* idx = next_temp(ctx);
            if (!emit_expr(ctx, node->children[0], arr, err, errcap) || !emit_expr(ctx, node->children[1], idx, err, errcap)) {
                free(arr);
                free(idx);
                return false;
            }
            emit_line(ctx, "RuntimeValue %s = make_null_value();", out_name);
            emit_line(ctx, "if (%s.type == RUNTIME_VALUE_ARRAY && %s.type == RUNTIME_VALUE_INT && %s.int_val >= 0 && (size_t)%s.int_val < %s.array_val.count) {", arr, idx, idx, idx, arr);
            ctx->indent++;
            emit_line(ctx, "%s = %s.array_val.elements[%s.int_val];", out_name, arr, idx);
            ctx->indent--;
            emit_line(ctx, "}");
            free(arr);
            free(idx);
            return true;
        }
        case AST_ARRAY_LITERAL:
            return emit_array_literal(ctx, node, out_name, err, errcap);
        default:
            set_error(err, errcap, "AOT: unsupported expression node type %d", (int)node->type);
            return false;
    }
}

static bool emit_assignment(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    if (node->child_count < 2) {
        set_error(err, errcap, "AOT: invalid assignment node");
        return false;
    }

    char* rhs = next_temp(ctx);
    if (!emit_expr(ctx, node->children[1], rhs, err, errcap)) {
        free(rhs);
        return false;
    }

    const ASTNode* lhs = node->children[0];
    if (lhs->type == AST_IDENTIFIER && lhs->operator_) {
        if (!node->operator_ || strcmp(node->operator_, "=") == 0) {
            emit_line(ctx, "env_set_var(env, \"%s\", %s);", lhs->operator_, rhs);
        } else {
            char* cur = next_temp(ctx);
            emit_line(ctx, "RuntimeValue %s = env_get_var(env, \"%s\");", cur, lhs->operator_);
            emit_line(ctx, "RuntimeValue __newv = make_null_value();");
            if (strcmp(node->operator_, "+=") == 0) emit_line(ctx, "__newv = make_int_value(%s.int_val + %s.int_val);", cur, rhs);
            else if (strcmp(node->operator_, "-=") == 0) emit_line(ctx, "__newv = make_int_value(%s.int_val - %s.int_val);", cur, rhs);
            else if (strcmp(node->operator_, "*=") == 0) emit_line(ctx, "__newv = make_int_value(%s.int_val * %s.int_val);", cur, rhs);
            else if (strcmp(node->operator_, "/=") == 0) emit_line(ctx, "__newv = make_int_value(%s.int_val / %s.int_val);", cur, rhs);
            else if (strcmp(node->operator_, "%=") == 0) emit_line(ctx, "__newv = make_int_value(%s.int_val %% %s.int_val);", cur, rhs);
            else emit_line(ctx, "__newv = %s;", rhs);
            emit_line(ctx, "env_set_var(env, \"%s\", __newv);", lhs->operator_);
            free(cur);
        }
        free(rhs);
        return true;
    }

    if (lhs->type == AST_ARRAY_ACCESS && lhs->child_count == 2) {
        char* arr = next_temp(ctx);
        char* idx = next_temp(ctx);
        if (!emit_expr(ctx, lhs->children[0], arr, err, errcap) || !emit_expr(ctx, lhs->children[1], idx, err, errcap)) {
            free(rhs);
            free(arr);
            free(idx);
            return false;
        }
        emit_line(ctx, "if (%s.type == RUNTIME_VALUE_ARRAY && %s.type == RUNTIME_VALUE_INT && %s.int_val >= 0 && (size_t)%s.int_val < %s.array_val.count) {", arr, idx, idx, idx, arr);
        ctx->indent++;
        emit_line(ctx, "%s.array_val.elements[%s.int_val] = %s;", arr, idx, rhs);
        ctx->indent--;
        emit_line(ctx, "}");
        free(arr);
        free(idx);
        free(rhs);
        return true;
    }

    free(rhs);
    set_error(err, errcap, "AOT: unsupported assignment target");
    return false;
}

static bool emit_block(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    for (size_t i = 0; i < node->child_count; i++) {
        if (!emit_stmt(ctx, node->children[i], err, errcap)) return false;
    }
    return true;
}

static bool emit_if(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    if (node->child_count < 2) {
        set_error(err, errcap, "AOT: invalid if node");
        return false;
    }
    char* cond = next_temp(ctx);
    if (!emit_expr(ctx, node->children[0], cond, err, errcap)) {
        free(cond);
        return false;
    }
    emit_line(ctx, "if ((%s.type == RUNTIME_VALUE_BOOL && %s.bool_val) || (%s.type == RUNTIME_VALUE_INT && %s.int_val != 0) || (%s.type == RUNTIME_VALUE_FLOAT && %s.float_val != 0.0)) {", cond, cond, cond, cond, cond, cond);
    ctx->indent++;
    if (!emit_stmt(ctx, node->children[1], err, errcap)) { free(cond); return false; }
    ctx->indent--;
    if (node->child_count > 2) {
        emit_line(ctx, "} else {");
        ctx->indent++;
        if (!emit_stmt(ctx, node->children[2], err, errcap)) { free(cond); return false; }
        ctx->indent--;
        emit_line(ctx, "}");
    } else {
        emit_line(ctx, "}");
    }
    free(cond);
    return true;
}

static bool emit_while(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    if (node->child_count < 2) {
        set_error(err, errcap, "AOT: invalid while node");
        return false;
    }
    emit_line(ctx, "while (1) {");
    ctx->indent++;
    char* cond = next_temp(ctx);
    if (!emit_expr(ctx, node->children[0], cond, err, errcap)) {
        free(cond);
        return false;
    }
    emit_line(ctx, "if (!((%s.type == RUNTIME_VALUE_BOOL && %s.bool_val) || (%s.type == RUNTIME_VALUE_INT && %s.int_val != 0) || (%s.type == RUNTIME_VALUE_FLOAT && %s.float_val != 0.0))) break;", cond, cond, cond, cond, cond, cond);
    free(cond);
    if (!emit_stmt(ctx, node->children[1], err, errcap)) return false;
    ctx->indent--;
    emit_line(ctx, "}");
    return true;
}

static bool emit_for(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    if (node->child_count < 3) {
        set_error(err, errcap, "AOT: invalid for node");
        return false;
    }
    char* start = next_temp(ctx);
    char* end = next_temp(ctx);
    if (!emit_expr(ctx, node->children[0], start, err, errcap) || !emit_expr(ctx, node->children[1], end, err, errcap)) {
        free(start);
        free(end);
        return false;
    }
    char* iter = next_temp(ctx);
    emit_line(ctx, "for (long %s = %s.int_val; %s < %s.int_val; %s++) {", iter, start, iter, end, iter);
    ctx->indent++;
    if (!emit_stmt(ctx, node->children[2], err, errcap)) {
        free(start);
        free(end);
        free(iter);
        return false;
    }
    ctx->indent--;
    emit_line(ctx, "}");
    free(start);
    free(end);
    free(iter);
    return true;
}

static bool emit_switch(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    if (node->child_count < 1) {
        set_error(err, errcap, "AOT: invalid switch node");
        return false;
    }
    char* sw = next_temp(ctx);
    if (!emit_expr(ctx, node->children[0], sw, err, errcap)) {
        free(sw);
        return false;
    }
    emit_line(ctx, "int __matched = 0;");
    for (size_t i = 1; i < node->child_count; i++) {
        const ASTNode* c = node->children[i];
        if (c->type == AST_WHEN && c->child_count >= 1) {
            char* w = next_temp(ctx);
            if (!emit_expr(ctx, c->children[0], w, err, errcap)) { free(w); free(sw); return false; }
            emit_line(ctx, "if (!__matched && %s.int_val == %s.int_val) {", sw, w);
            ctx->indent++;
            emit_line(ctx, "__matched = 1;");
            for (size_t j = 1; j < c->child_count; j++) {
                if (!emit_stmt(ctx, c->children[j], err, errcap)) { free(w); free(sw); return false; }
            }
            ctx->indent--;
            emit_line(ctx, "}");
            free(w);
        } else if (c->type == AST_DEFAULT) {
            emit_line(ctx, "if (!__matched) {");
            ctx->indent++;
            for (size_t j = 0; j < c->child_count; j++) {
                if (!emit_stmt(ctx, c->children[j], err, errcap)) { free(sw); return false; }
            }
            ctx->indent--;
            emit_line(ctx, "}");
        }
    }
    free(sw);
    return true;
}

static bool emit_return(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    if (!ctx->in_function) {
        if (node->child_count > 0) {
            char* rv = next_temp(ctx);
            if (!emit_expr(ctx, node->children[0], rv, err, errcap)) { free(rv); return false; }
            emit_line(ctx, "(void)%s;", rv);
            free(rv);
        }
        return true;
    }

    if (node->child_count > 0) {
        char* rv = next_temp(ctx);
        if (!emit_expr(ctx, node->children[0], rv, err, errcap)) { free(rv); return false; }
        emit_line(ctx, "__ret = %s;", rv);
        free(rv);
    }
    emit_line(ctx, "free_environment(env);");
    emit_line(ctx, "return __ret;");
    return true;
}

static bool emit_stmt(AOTCtx* ctx, const ASTNode* node, char* err, size_t errcap) {
    if (!node) return true;

    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            return emit_block(ctx, node, err, errcap);
        case AST_ASSIGNMENT:
            return emit_assignment(ctx, node, err, errcap);
        case AST_IF_STATEMENT:
            return emit_if(ctx, node, err, errcap);
        case AST_WHILE_STATEMENT:
            return emit_while(ctx, node, err, errcap);
        case AST_FOR_STATEMENT:
            return emit_for(ctx, node, err, errcap);
        case AST_SWITCH:
            return emit_switch(ctx, node, err, errcap);
        case AST_RETURN_STATEMENT:
            return emit_return(ctx, node, err, errcap);
        case AST_FUNCTION_DECLARATION:
            set_error(err, errcap, "AOT: user-defined function declarations are not yet supported");
            return false;
        case AST_FUNCTION_CALL: {
            char* out = next_temp(ctx);
            bool ok = emit_call(ctx, node, out, err, errcap);
            free(out);
            return ok;
        }
        case AST_BREAK:
            emit_line(ctx, "break;");
            return true;
        case AST_CONTINUE:
            emit_line(ctx, "continue;");
            return true;
        case AST_COMMENT:
            return true;
        default: {
            char* tmp = next_temp(ctx);
            bool ok = emit_expr(ctx, node, tmp, err, errcap);
            free(tmp);
            return ok;
        }
    }
}

static bool write_c_file(const ASTNode* root, const char* c_path, char* err, size_t errcap) {
    FILE* out = fopen(c_path, "w");
    if (!out) {
        set_error(err, errcap, "AOT: failed to create %s", c_path);
        return false;
    }

    AOTCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.out = out;

    fputs("#include <stdio.h>\n", out);
    fputs("#include <stdlib.h>\n", out);
    fputs("#include <string.h>\n", out);
    fputs("#include \"runtimeEnv.h\"\n", out);
    fputs("#include \"runtimeValue.h\"\n\n", out);

    fputs("int main(void) {\n", out);
    ctx.indent = 1;
    emit_line(&ctx, "RuntimeEnvironment* env = create_environment(NULL);");
    emit_line(&ctx, "built_in_functions(env);");

    if (!emit_stmt(&ctx, root, err, errcap)) {
        fclose(out);
        return false;
    }

    emit_line(&ctx, "free_environment(env);");
    emit_line(&ctx, "return 0;");
    fputs("}\n", out);

    fclose(out);
    return true;
}

bool aot_compile_ast(const ASTNode* root, const char* output_binary, char* error_buffer, size_t error_capacity) {
    if (!root || !output_binary) {
        set_error(error_buffer, error_capacity, "AOT: invalid arguments");
        return false;
    }

    char c_path[1024];
    snprintf(c_path, sizeof(c_path), "%s.c", output_binary);

    if (!write_c_file(root, c_path, error_buffer, error_capacity)) {
        return false;
    }

#ifdef _WIN32
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "gcc -O3 -Iinclude \"%s\" src/runtimeEnv.c src/runtimeValue.c -o \"%s\"", c_path, output_binary);
#else
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cc -O3 -Iinclude \"%s\" src/runtimeEnv.c src/runtimeValue.c -o \"%s\"", c_path, output_binary);
#endif

    int rc = system(cmd);
    if (rc != 0) {
        set_error(error_buffer, error_capacity, "AOT: native compile failed (cmd rc=%d)", rc);
        return false;
    }

    return true;
}
