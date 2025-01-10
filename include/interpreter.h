/***********************************************************
* File: interpreter.h
* This file contains the interpreter for the Clock coding language.
* The interpreter is responsible for executing the AST.
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/




#pragma once
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "runtimeEnv.h"


/**
 * Evaluate (execute) the AST starting from the given node.
 * Returns a RuntimeValue, which might be null/void for statements that don't produce a value.
 */
RuntimeValue eval_ast_node(ASTNode* node, RuntimeEnvironment* env);

/**
 * theses functions are responsible for evaluating the different types of nodes in the AST.
 * They are called by eval_ast_node.
 */
RuntimeValue eval_program(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_block(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_literal(ASTNode* node);
RuntimeValue eval_identifier(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_assignment(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_if_statement(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_while_statement(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_for_statement(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_binary_expr(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_unary_expr(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_function_call(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue evaluate_comparison(const char* op, RuntimeValue leftVal, RuntimeValue rightVal);
RuntimeValue* collect_arguments(ASTNode* argsNode, RuntimeEnvironment* env, size_t* out_count);
RuntimeValue eval_condition(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue make_special_value(const char* special);
RuntimeValue make_array_value(RuntimeValue* elements, size_t count);
RuntimeValue eval_array_literal(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_array_access(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue apply_compound_operator(const char* op, RuntimeValue leftVal, RuntimeValue rightVal);
RuntimeValue eval_function_declaration(ASTNode* node, RuntimeEnvironment* env);
RuntimeValue eval_user_function_call(RuntimeValue functionVal, RuntimeValue* args, size_t arg_count);
ASTNode* create_param_list_node(ASTNode** paramList, size_t paramCount);
RuntimeValue convert_return_val_to_datatype(RuntimeValue value);
RuntimeValue eval_switch_statement(ASTNode* node, RuntimeEnvironment* env);

// Helper functions for switch statements
RuntimeValue eval_when_case(ASTNode* caseNode, RuntimeValue switchValue, RuntimeEnvironment* env);
RuntimeValue eval_default_case(ASTNode* defaultNode, RuntimeEnvironment* env);

// Helper functions for return statements
ASTNode* find_next_node_after_return(ASTNode* returnNode);

// Helper function to print master return value
void print_return(RuntimeEnvironment* env);


/**
 * The main entry point for interpreting the entire AST program.
 * Creates an environment, evaluates the root node (usually AST_PROGRAM),
 * and then cleans up.
 */
void interpret(ASTNode* root);



#endif // INTERPRETER_H

