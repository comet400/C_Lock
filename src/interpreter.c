/***********************************************************
* File: interpreter.c
* This file contains the interpreter for the Clock coding language.
* The interpreter is responsible for executing the AST.
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Interpreter.h"  




/***********************************************************
* Function: print_return
* Description: the function prints the return value of the program.
* Parameters: RuntimeEnvironment* env
* Return: Void
* ***********************************************************/
void print_return(RuntimeEnvironment* env) {

    // change print color to vibrant yellow
    printf("\033[0;93m\n");
    if (env->function_returned) {
        printf("Clock Returned: ");
        if (env->return_value.type == RUNTIME_VALUE_INT) {
            printf("%ld\n", env->return_value.int_val);
        }
        else if (env->return_value.type == RUNTIME_VALUE_FLOAT) {
            printf("%f\n", env->return_value.float_val);
        }
        else if (env->return_value.type == RUNTIME_VALUE_BOOL) {
            printf("%s\n", env->return_value.bool_val ? "true" : "false");
        }
        else if (env->return_value.type == RUNTIME_VALUE_STRING) {
            printf("%s\n", env->return_value.string_val);
        }
        else if (env->return_value.type == RUNTIME_VALUE_NULL) {
            printf("null\n");
        }
        else {
            printf("Unknown return type\n");
        }
        printf("\n");
    }
    // back to default color
    printf("\033[0m\n");
}



/***********************************************************
* Function: eval_identifier_variable
* Description: This function evaluates the identifier as a variable.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
***********************************************************/
RuntimeValue eval_identifier_variable(ASTNode* node, RuntimeEnvironment* env) {
    if (!node || !env) {
        fprintf(stderr, "Invalid arguments provided to eval_identifier_variable.\n");
        return make_null_value();
    }

    const char* varName = node->operator_;  // The identifier name

    // Search for the variable in the environment
    RuntimeValue value = env_get_var(env, varName);
	if (value.type != RUNTIME_VALUE_NULL) {
		return value;
	}

    // Variable not found
    fprintf(stderr, "Variable '%s' not found in the current environment.\n", varName);
    return make_null_value();
}





/***********************************************************
* Function: eval_function_identifier
* Description: This function evaluates the identifier as a function.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
***********************************************************/
RuntimeValue eval_function_identifier(ASTNode* node, RuntimeEnvironment* env) {
    if (!node || !env) {
        fprintf(stderr, "Invalid arguments provided to eval_function_identifier.\n");
        return make_null_value();
    }

    const char* funcName = node->operator_;  // The identifier name

    // Search for the function in the environment
    RuntimeValue value = env_get_func(env, funcName);
	if (value.type != RUNTIME_VALUE_NULL) {
		return value;
	}

    // Function not found
    fprintf(stderr, "Function '%s' not found in the current environment.\n", funcName);
    return make_null_value();
}


/***********************************************************
* Function: interpret
* Description: this function interprets the whole program.
* Parameters: ASTNode* root
* Return: Void
* ***********************************************************/
void interpret(ASTNode* root) {
    // Create a global environment (hash table or similar)
    RuntimeEnvironment* globalEnv = create_environment(NULL);
    built_in_functions(globalEnv);

    // Evaluate the top-level AST (AST_PROGRAM).
    eval_ast_node(root, globalEnv);

    // environment return value
    print_return(globalEnv);

    free(globalEnv);
}




/***********************************************************
* Function: make_special_value
* Description: this function prepares a runtime value of type special (return, stop).
* Parameters: const char* special
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue make_special_value(const char* special) {
    RuntimeValue value = make_null_value();
    value.type = RUNTIME_VALUE_SPECIAL;
    value.special_val = special;
    return value;
}




/***********************************************************
* Function: eval_ast_node
* Description: this function evaluates the AST node.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_ast_node(ASTNode* node, RuntimeEnvironment* env) {
    if (!node) {
        return make_null_value();
    }

    if (env->function_returned) {
        return env->return_value;
    }
    switch (node->type) {
    case AST_PROGRAM:
        return eval_program(node, env); // Evaluate the program

    case AST_BLOCK:
        return eval_block(node, env); // Evaluate blocks

    case AST_LITERAL:
        return eval_literal(node); // Evaluate literals

    case AST_IDENTIFIER:
		if (env->is_Function) {
			return eval_function_identifier(node, env); // Evaluate function identifiers
		}
		else {
			return eval_identifier_variable(node, env); // Evaluate variable identifiers
		}

    case AST_ASSIGNMENT:
		env->is_Function = false;
        return eval_assignment(node, env); // Evaluate assignments

    case AST_IF_STATEMENT:
        return eval_if_statement(node, env); // Evaluate if statements

    case AST_WHILE_STATEMENT:
        return eval_while_statement(node, env); // Evaluate while statements

    case AST_FOR_STATEMENT:
        return eval_for_statement(node, env); // Evaluate for statements

    case AST_BINARY_EXPR:
        return eval_binary_expr(node, env); // Evaluate binary expressions

    case AST_UNARY_EXPR:
        return eval_unary_expr(node, env);  // Evaluate unary expressions

    case AST_ARRAY_LITERAL:
        return eval_array_literal(node, env); // Evaluate array literals

    case AST_ARRAY_ACCESS:
        return eval_array_access(node, env); // Evaluate array access

    case AST_BREAK:
        return make_special_value("stop"); // Stop the current loop

    case AST_FUNCTION_CALL:
		env->is_Function = true;
        return eval_function_call(node, env); // Evaluate function calls

    case AST_FUNCTION_DECLARATION:
		env->is_Function = true;
        return eval_function_declaration(node, env); // Evaluate function declarations

    case AST_RETURN_STATEMENT:
        if (env->parent == NULL) {
            ASTNode* returnExpressionNode = find_next_node_after_return(node);
            RuntimeValue resultValue = eval_ast_node(returnExpressionNode, env);
            env->return_value = resultValue;
            env->function_returned = true;
            return resultValue;           // Return the result of the expression
        }
        return make_special_value("return"); // Return from the current function

    case AST_SWITCH:
        return eval_switch_statement(node, env); // Evaluate switch statements

    default:
        fprintf(stderr, "Error: Unsupported AST node type: %d\n", node->type);
        return make_null_value();
    }
}





/***********************************************************
* Function: eval_when_case
* Description: this function evaluates the when case of switch statement.
* Parameters: ASTNode* caseNode, RuntimeValue switchValue, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_when_case(ASTNode* caseNode, RuntimeValue switchValue, RuntimeEnvironment* env) {
    // Evaluate the "when" condition
    RuntimeValue caseValue = eval_ast_node(caseNode->children[0], env);

    // Check if the "when" condition matches the switch value
    if (switchValue.int_val == caseValue.int_val) {
        // Execute the statements in the matching "when"
        for (size_t i = 1; i < caseNode->child_count; i++) {
            RuntimeValue result = eval_ast_node(caseNode->children[i], env);

            // If "stop" is encountered, terminate this case
            if (result.type == RUNTIME_VALUE_SPECIAL && strcmp(result.special_val, "stop") == 0) {
                break;
            }

            // Return the result of the case
            result = make_special_value("when");
            return result;
        }
    }

    // No match or no value returned
    return make_null_value();
}




/***********************************************************
* Function: eval_default_case
* Description: this function evaluates the default case of switch statement.
* Parameters: ASTNode* defaultNode, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_default_case(ASTNode* defaultNode, RuntimeEnvironment* env) {

    // Execute the statements in the default case
    for (size_t i = 0; i < defaultNode->child_count; i++) {
        RuntimeValue result = eval_ast_node(defaultNode->children[i], env);

        // If "stop" is encountered, terminate the default case
        if (result.type == RUNTIME_VALUE_SPECIAL && strcmp(result.special_val, "stop") == 0) {
            break;
        }

        // Return the result of the default case
        return result;
    }

    // No value returned from default
    return make_null_value();
}




/***********************************************************
* Function: eval_switch_statement
* Description: this function evaluates the switch statement.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_switch_statement(ASTNode* node, RuntimeEnvironment* env) {

    // Evaluate the switch expression
    RuntimeValue switchValue = eval_ast_node(node->children[0], env);
    bool hasDefault = true;
    // Traverse the "when" cases
    for (size_t i = 1; i < node->child_count; i++) {
        ASTNode* caseNode = node->children[i];

        if (caseNode->type == AST_WHEN) {
            // Evaluate the "when" case
            RuntimeValue result = eval_when_case(caseNode, switchValue, env);
            if (result.type != RUNTIME_VALUE_NULL) {
                hasDefault = false;
                return result; // Return the result if a value is produced
            }
        }
        else if (caseNode->type == AST_DEFAULT && hasDefault) {
            // Evaluate the default case
            return eval_default_case(caseNode, env);
        }
    }

    // If no cases matched and no default exists, return null
    return make_null_value();
}






/***********************************************************
* Function: eval_function_call
* Description: this function evaluates the function call.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_function_call(ASTNode* node, RuntimeEnvironment* env) {
    if (node->child_count < 1) {
        fprintf(stderr, "Runtime Error: No function specified.\n");
        return make_null_value();
    }

    // Evaluate the function identifier in the current env (not the parent!)
    ASTNode* functionIdentNode = node->children[0];
    RuntimeValue functionVal = eval_ast_node(functionIdentNode, env);


    if (functionVal.type == RUNTIME_VALUE_NULL) {
        fprintf(stderr, "Runtime Error: Function not found.\n");
        return make_null_value();
    }

    if (functionVal.type != RUNTIME_VALUE_BUILTIN &&
        functionVal.type != RUNTIME_VALUE_FUNCTION)
    {
        fprintf(stderr, "Runtime Error: Attempt to call a non-function.\n");
        return make_null_value();
    }

    // Collect arguments (also from the current env, so we can use local vars!)
    size_t arg_count = 0;
    RuntimeValue* args = NULL;
    if (node->child_count > 1) {
        ASTNode* argsNode = node->children[1];
        args = collect_arguments(argsNode, env, &arg_count);
        if (!args) {
            fprintf(stderr, "Runtime Error: Failed to evaluate arguments.\n");
            return make_null_value();
        }
    }

    // Dispatch user function vs builtin
    if (functionVal.type == RUNTIME_VALUE_FUNCTION) {
        return eval_user_function_call(functionVal, args, arg_count);
    }
    else if (functionVal.type == RUNTIME_VALUE_BUILTIN) {
        RuntimeValue result = functionVal.builtin_val.fn(args, arg_count);
        free(args);
        return result;
    }

    return make_null_value();
}




/***********************************************************
* Function: create_param_list_node
* Description: this function creates a parameter list node.
* Parameters: ASTNode** paramList, size_t paramCount
* Return: ASTNode*
* ***********************************************************/
ASTNode* create_param_list_node(ASTNode** paramList, size_t paramCount) {
    ASTNode* paramNode = (ASTNode*)malloc(sizeof(ASTNode));
    if (!paramNode) {
        fprintf(stderr, "Memory allocation failed in create_param_list_node\n");
        exit(EXIT_FAILURE);
    }
    paramNode->type = AST_PARAMETER_LIST;
    paramNode->child_count = paramCount;
    paramNode->children = paramList;
    // set other fields to defaults (e.g., operator_ = NULL, etc.)
    return paramNode;
}




/***********************************************************
* Function: eval_function_declaration
* Description: this function evaluates the function declaration.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_function_declaration(ASTNode* node, RuntimeEnvironment* env) {
    if (!node || node->child_count < 2) {
        fprintf(stderr, "Error: Invalid function declaration.\n");
        return make_null_value();
    }

    // child[0] = function name
    ASTNode* functionIdentNode = node->children[0];
    const char* functionName = (functionIdentNode && functionIdentNode->operator_)
        ? functionIdentNode->operator_
        : NULL;

    if (!functionName) {
        fprintf(stderr, "Error: Function name is missing or invalid.\n");
        return make_null_value();
    }

    // The LAST child is always the body (AST_BLOCK).
    ASTNode* bodyNode = node->children[node->child_count - 1];

    // Everything between [1..child_count-2] are parameter nodes
    size_t paramCount = (node->child_count >= 2) ? node->child_count - 2 : 0; // Ensure paramCount is non-negative
    ASTNode** paramList = NULL;

    if (paramCount > 0) {
        paramList = (ASTNode**)malloc(sizeof(ASTNode*) * paramCount);
        if (!paramList) {
            fprintf(stderr, "Memory allocation failed in eval_function_declaration.\n");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < paramCount; i++) {
            // Ensure we don't access beyond bounds of node->children
            paramList[i] = (1 + i < node->child_count) ? node->children[1 + i] : NULL;
        }
    }

    // Create a node for the parameter list
    ASTNode* paramsNode = create_param_list_node(paramList, paramCount);

    // Build the RuntimeValue for the user function
    RuntimeValue functionValue;
    functionValue.type = RUNTIME_VALUE_FUNCTION;
    functionValue.function_val.fn = NULL;  // user-defined function pointer is null
    functionValue.function_val.env = env;  // capture current env (closure)
    functionValue.function_val.body = bodyNode ? bodyNode : NULL;
    functionValue.function_val.parameters = paramsNode ? paramsNode : NULL;

    // Insert into the environment
	env_set_func(env, functionName, functionValue);

    return make_null_value();
}




/***********************************************************
* Function: eval_user_function_call
* Description: this function evaluates the user function call.
* Parameters: RuntimeValue functionVal, RuntimeValue* args, size_t arg_count
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_user_function_call(RuntimeValue functionVal, RuntimeValue* args, size_t arg_count) {
    // 1) Create a new environment
    RuntimeEnvironment* functionEnv = create_environment(128);
    functionEnv->parent = functionVal.function_val.env;

    // 2) Use the parameter list stored in functionVal
    ASTNode* paramList = functionVal.function_val.parameters;
    if (!paramList) {
        fprintf(stderr, "Error: Function parameters are missing.\n");
        free(functionEnv);
        return make_null_value();
    }

    // Bind each parameter to the corresponding argument
    size_t paramCount = paramList->child_count;
    for (size_t i = 0; i < arg_count && i < paramCount; i++) {
        ASTNode* paramNode = paramList->children[i];
        const char* paramName = paramNode->operator_;
        if (!paramName) {
            fprintf(stderr, "Error: Parameter name is missing.\n");
            free(functionEnv);
            return make_null_value();
        }
		env_set_func(functionEnv, paramName, args[i]);
    }

    // 3) Evaluate the body in the new environment
    RuntimeValue result = eval_ast_node(functionVal.function_val.body, functionEnv);

    // 4) Clean up
    free(functionEnv);

    // If there's no explicit return, 'result' is likely null from 'eval_block(...)'
    return result;
}





/***********************************************************
* Function: eval_array_literal
* Description: this function evaluates the array literal.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_array_literal(ASTNode* node, RuntimeEnvironment* env) {
    if (!node || node->type != AST_ARRAY_LITERAL) {
        fprintf(stderr, "Error: Invalid node for array literal evaluation.\n");
        return make_null_value();
    }

    // Count the number of elements
    size_t count = 0;
    ASTNode* current = node->children[0];

    while (current && current->type == AST_BINARY_EXPR && strcmp(current->operator_, ",") == 0) {
        count++;
        current = current->children[0]; // Move left in the binary expression
    }
    if (current) count++; // Include the last element

    // Allocate space for array elements
    RuntimeValue* elements = (RuntimeValue*)malloc(count * sizeof(RuntimeValue));
    if (!elements) {
        fprintf(stderr, "Error: Memory allocation failed for array.\n");
        exit(EXIT_FAILURE);
    }

    // Traverse the AST to populate the array
    current = node->children[0];
    for (size_t i = count; i > 0; i--) {
        if (current->type == AST_BINARY_EXPR && strcmp(current->operator_, ",") == 0) {
            elements[i - 1] = eval_ast_node(current->children[1], env); // Right child
            current = current->children[0]; // Move left
        }
        else if (current->type == AST_LITERAL) {
            elements[i - 1] = eval_ast_node(current, env); // Last element
        }
        else {
            fprintf(stderr, "Error: Unexpected node type in array literal.\n");
            free(elements);
            return make_null_value();
        }
    }

    // Create a RuntimeValue for the array
    return make_array_value(elements, count);
}




/***********************************************************
* Function: apply_compound_operator
* Description: this function applies the compound operator.
* Parameters: const char* op, RuntimeValue leftVal, RuntimeValue rightVal
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue apply_compound_operator(const char* op, RuntimeValue leftVal, RuntimeValue rightVal) {
    if (leftVal.type != rightVal.type) {
        fprintf(stderr, "Runtime Error: Type mismatch in compound assignment.\n");
        return make_null_value();
    }

    if (leftVal.type == RUNTIME_VALUE_INT) {
        long left = leftVal.int_val;
        long right = rightVal.int_val;

        if (strcmp(op, "+=") == 0) return make_int_value(left + right);
        if (strcmp(op, "-=") == 0) return make_int_value(left - right);
        if (strcmp(op, "*=") == 0) return make_int_value(left * right);
        if (strcmp(op, "/=") == 0) {
            if (right == 0) {
                fprintf(stderr, "Runtime Error: Division by zero.\n");
                return make_null_value();
            }
            return make_int_value(left / right);
        }
        if (strcmp(op, "%=") == 0) {
            if (right == 0) {
                fprintf(stderr, "Runtime Error: Modulo by zero.\n");
                return make_null_value();
            }
            return make_int_value(left % right);
        }
    }

    if (leftVal.type == RUNTIME_VALUE_FLOAT) {
        double left = leftVal.float_val;
        double right = rightVal.float_val;

        if (strcmp(op, "+=") == 0) return make_float_value(left + right);
        if (strcmp(op, "-=") == 0) return make_float_value(left - right);
        if (strcmp(op, "*=") == 0) return make_float_value(left * right);
        if (strcmp(op, "/=") == 0) {
            if (right == 0.0) {
                fprintf(stderr, "Runtime Error: Division by zero.\n");
                return make_null_value();
            }
            return make_float_value(left / right);
        }
    }

    fprintf(stderr, "Runtime Error: Unsupported operator '%s' for type.\n", op);
    return make_null_value();
}




/***********************************************************
* Function: convert_return_val_to_datatype
* Description: this function converts the return value to a specific data type.
* Parameters: RuntimeValue value
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue convert_return_val_to_datatype(RuntimeValue value) {
    // Determine the type of the RuntimeValue and process it accordingly
    switch (value.type) {
    case RUNTIME_VALUE_INT: {
        long intVal = value.int_val;
        return make_int_value(intVal);
    }

    case RUNTIME_VALUE_FLOAT: {
        double floatVal = value.float_val;
        return make_float_value(floatVal);
    }

    case RUNTIME_VALUE_BOOL: {
        bool boolVal = value.bool_val;
        return make_bool_value(boolVal);
    }

    case RUNTIME_VALUE_STRING: {
        if (value.string_val) {
            return make_string_value(value.string_val);
        }
        else {
            fprintf(stderr, "Error: Null string in RuntimeValue.\n");
            return make_null_value();
        }
    }

    case RUNTIME_VALUE_ARRAY: {
        if (value.array_val.elements) {
            return make_array_value(value.array_val.elements, value.array_val.count);
        }
        else {
            fprintf(stderr, "Error: Null array in RuntimeValue.\n");
            return make_null_value();
        }
    }

    case RUNTIME_VALUE_NULL:
        return make_null_value();

    default:
        fprintf(stderr, "Error: Unsupported RuntimeValue type in convert_return_val_to_datatype.\n");
        printf("Type: %d\n", value.type);
        return make_null_value();
    }
}




/***********************************************************
* Function: eval_assignment
* Description: this function evaluates the assignment.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_assignment(ASTNode* node, RuntimeEnvironment* env) {
    if (node->child_count < 2) {
        fprintf(stderr, "Error: Invalid assignment.\n");
        return make_null_value();
    }

    ASTNode* leftNode = node->children[0];
    ASTNode* rightNode = node->children[1];
    const char* op = node->operator_; // Assignment operator (e.g., "=", "+=", "-=")

    RuntimeValue rightVal = eval_ast_node(rightNode, env);

    if (rightVal.type == RUNTIME_VALUE_SPECIAL &&
        strcmp(rightVal.special_val, "return") == 0) {
        if (rightVal.return_val) {
            rightVal = env->return_value;
        }
        else {
            fprintf(stderr, "Error: Invalid return value in assignment.\n");
            return make_null_value();
        }
    }

    if (rightVal.type == AST_FUNCTION_CALL) {
        rightVal = eval_user_function_call(rightVal, NULL, 0);
    }

    if (leftNode->type == AST_ARRAY_ACCESS) {
        // Handle array element assignment
        ASTNode* arrayNode = leftNode->children[0];
        ASTNode* indexNode = leftNode->children[1];

        RuntimeValue arrayVal = eval_ast_node(arrayNode, env);
        RuntimeValue indexVal = eval_ast_node(indexNode, env);

        if (arrayVal.type != RUNTIME_VALUE_ARRAY) {
            fprintf(stderr, "Error: Variable is not an array.\n");
            return make_null_value();
        }
        if (indexVal.type != RUNTIME_VALUE_INT) {
            fprintf(stderr, "Error: Array index must be an integer.\n");
            return make_null_value();
        }

        long index = indexVal.int_val;
        if (index < 0 || index >= arrayVal.array_val.count) {
            fprintf(stderr, "Error: Array index out of bounds.\n");
            return make_null_value();
        }

        RuntimeValue* targetVal = &arrayVal.array_val.elements[index];

        if (strcmp(op, "=") == 0) {
            *targetVal = rightVal; // Simple assignment
        }
        else {
            *targetVal = apply_compound_operator(op, *targetVal, rightVal);
        }
        return *targetVal;
    }
    else if (leftNode->type == AST_IDENTIFIER) {
        // Handle normal variable assignment
        const char* varName = leftNode->operator_;


        if (strcmp(op, "=") == 0) {
			env_set_var(env, varName, rightVal); // Simple assignment
        }
        else {
            RuntimeValue currentVal = env_get_var(env, varName);
            RuntimeValue newVal = apply_compound_operator(op, currentVal, rightVal);
			env_set_var(env, varName, newVal);
        }
        return rightVal;
    }

    fprintf(stderr, "Error: Invalid assignment target.\n");
    return make_null_value();
}




/***********************************************************
* Function: eval_array_access
* Description: this function evaluates the array access.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_array_access(ASTNode* node, RuntimeEnvironment* env) {
    if (node->type != AST_ARRAY_ACCESS || node->child_count != 2) {
        fprintf(stderr, "Error: Invalid array access node.\n");
        return make_null_value();
    }

    // Evaluate the array identifier
    ASTNode* arrayNode = node->children[0];
    RuntimeValue arrayVal = eval_ast_node(arrayNode, env);
    if (arrayVal.type != RUNTIME_VALUE_ARRAY) {
        fprintf(stderr, "Error: Variable is not an array.\n");
        return make_null_value();
    }

    // Evaluate the index
    ASTNode* indexNode = node->children[1];
    RuntimeValue indexVal = eval_ast_node(indexNode, env);
    if (indexVal.type != RUNTIME_VALUE_INT) {
        fprintf(stderr, "Error: Array index must be an integer.\n");
        return make_null_value();
    }

    // Check index bounds
    long index = indexVal.int_val;
    if (index < 0 || index >= arrayVal.array_val.count) {
        fprintf(stderr, "Error: Array index out of bounds.\n");
        return make_null_value();
    }

    // Return the value at the specified index
    return arrayVal.array_val.elements[index];
}




/***********************************************************
* Function: make_array_value
* Description: this function prepares a runtime value of type array.
* Parameters: RuntimeValue* elements, size_t count
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue make_array_value(RuntimeValue* elements, size_t count) {
    RuntimeValue value;
    value.type = RUNTIME_VALUE_ARRAY;
    value.array_val.elements = elements;
    value.array_val.count = count;
    return value;
}




/***********************************************************
* Function: eval_program
* Description: this function evaluates the program.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_program(ASTNode* node, RuntimeEnvironment* env) {
    // Just evaluate each child in order. Typically AST_PROGRAM is the root.
    RuntimeValue lastVal = make_null_value();
    for (size_t i = 0; i < node->child_count; i++) {
        lastVal = eval_ast_node(node->children[i], env);
    }
    return lastVal;
}




/***********************************************************
* Function: collect_arguments
* Description: this function collects the arguments.
* Parameters: ASTNode* argsNode, RuntimeEnvironment* env, size_t* out_count
* Return: RuntimeValue*
* ***********************************************************/
RuntimeValue* collect_arguments(ASTNode* argsNode, RuntimeEnvironment* env, size_t* out_count) {
    if (!argsNode) {
        *out_count = 0;
        return NULL;
    }

    // Determine the number of arguments
    size_t arg_count = 0;
    ASTNode* current = argsNode;

    while (current->type == AST_BINARY_EXPR && strcmp(current->operator_, ",") == 0) {
        arg_count++;
        current = current->children[0]; // Move left in the binary expression
    }
    arg_count++; // Add the last argument

    // Allocate space for arguments
    RuntimeValue* args = (RuntimeValue*)malloc(arg_count * sizeof(RuntimeValue));
    if (!args) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Traverse again to evaluate arguments
    current = argsNode;
    env->is_Function = false;
    for (size_t i = arg_count; i > 0; i--) {
        if (current->type == AST_BINARY_EXPR && strcmp(current->operator_, ",") == 0) {
            args[i - 1] = eval_ast_node(current->children[1], env); // Right child
            current = current->children[0]; // Move left
        }
        else {
            args[i - 1] = eval_ast_node(current, env); // Final argument
        }
    }

    *out_count = arg_count;
    return args;
}




/***********************************************************
* Function: find_next_node_after_return
* Description: this function finds the next node after return.
* Parameters: ASTNode* returnNode
* Return: ASTNode*
* ***********************************************************/
ASTNode* find_next_node_after_return(ASTNode* returnNode) {
    if (!returnNode) return NULL;

    if (returnNode->type == AST_RETURN_STATEMENT) {
        return returnNode->children[0];
    }

    // Traverse all children of the `return` node
    for (size_t i = 0; i < returnNode->child_count; i++) {
        ASTNode* child = returnNode->children[i];

        // If the child is a return node, check for adjacent nodes
        if (child->type == AST_RETURN_STATEMENT) {
            // Ensure there's a next child
            if (0 < child->child_count) {
                return child->children[0]; // Return the next adjacent node
            }
            else {
                return NULL; // No adjacent node
            }
        }

        // Recursively search within this child
        ASTNode* result = find_next_node_after_return(child);
        if (result) return result; // Return the first valid result
    }

    return NULL; // No adjacent node found
}




/***********************************************************
* Function: eval_block
* Description: this function evaluates the block.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_block(ASTNode* node, RuntimeEnvironment* env) {
    if (!env->function_returned)
    {
        for (size_t i = 0; i < node->child_count; i++) {
            RuntimeValue result = eval_ast_node(node->children[i], env);

            // Propagate `break` signals (e.g., in loops)
            if (result.type == RUNTIME_VALUE_SPECIAL && strcmp(result.special_val, "stop") == 0) {
                return result;
            }

            // Propagate `return` signals
            if (result.type == RUNTIME_VALUE_SPECIAL && strcmp(result.special_val, "return") == 0) {
                if (node->children[0]->child_count > 0) {
                    ASTNode* returnExpressionNode = find_next_node_after_return(node);

                    // Evaluate the entire expression inside the return node
                    RuntimeValue resultValue = eval_ast_node(returnExpressionNode, env);
                    env->function_returned = true;
                    env->return_value = resultValue;
                    return resultValue;
                }

                // If no expression exists, return null
                fprintf(stderr, "Error: Return statement missing a value.\n");
                return make_null_value();
            }
        }
    }

    return env->return_value;
}






/***********************************************************
* Function: eval_literal
* Description: this function evaluates the literal.
* Parameters: ASTNode* node
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_literal(ASTNode* node) {
    switch (node->value_kind) {
    case VALUE_INT:
        return make_int_value(node->value.int_val);

    case VALUE_FLOAT:
        return make_float_value(node->value.float_val);

    case VALUE_BOOL:
        return make_bool_value(node->value.bool_val);

    case VALUE_STRING:
        return make_string_value(node->value.str_val);

    default:
        return make_null_value();
    }
}





/***********************************************************
* Function: eval_condition
* Description: this function evaluates the condition.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_condition(ASTNode* node, RuntimeEnvironment* env) {
    if (!node) return make_null_value();

    if (node->type == AST_BINARY_EXPR) {
        const char* op = node->operator_;
        ASTNode* leftNode = node->children[0];
        ASTNode* rightNode = node->children[1];

        RuntimeValue leftVal = eval_condition(leftNode, env);
        RuntimeValue rightVal = eval_condition(rightNode, env);

        if (strcmp(op, "&&") == 0) {
            bool left = (leftVal.type == RUNTIME_VALUE_BOOL && leftVal.bool_val);
            bool right = (rightVal.type == RUNTIME_VALUE_BOOL && rightVal.bool_val);
            return make_bool_value(left && right);
        }
        else if (strcmp(op, "||") == 0) {
            bool left = (leftVal.type == RUNTIME_VALUE_BOOL && leftVal.bool_val);
            bool right = (rightVal.type == RUNTIME_VALUE_BOOL && rightVal.bool_val);
            return make_bool_value(left || right);
        }
        else {
            // Fallback for other binary conditions like ==, !=, <, etc.
            return evaluate_comparison(op, leftVal, rightVal);
        }
    }

    // If it's a literal or a single condition, evaluate it normally
    return eval_ast_node(node, env);
}




/***********************************************************
* Function: eval_if_statement
* Description: this function evaluates the if statement.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_if_statement(ASTNode* node, RuntimeEnvironment* env) {
    if (node->child_count < 2) {
        fprintf(stderr, "Error: Invalid if statement. Missing condition or block.\n");
        return make_null_value();
    }

    // The first child is the condition node
    ASTNode* conditionNode = node->children[0];
    RuntimeValue condVal = eval_ast_node(conditionNode, env); // Evaluate the condition directly

    // Determine the truthiness of the condition
    bool isTrue = false;
    if (condVal.type == RUNTIME_VALUE_BOOL) {
        isTrue = condVal.bool_val; // Boolean true/false
    }
    else if (condVal.type == RUNTIME_VALUE_INT) {
        isTrue = (condVal.int_val != 0); // Non-zero integers are true
    }
    else if (condVal.type == RUNTIME_VALUE_FLOAT) {
        isTrue = (condVal.float_val != 0.0); // Non-zero floats are true
    }
    else {
        fprintf(stderr, "Error: Invalid condition type in if statement.\n");
        return make_null_value();
    }

    // Evaluate the appropriate block based on the condition
    if (isTrue) {
        return eval_ast_node(node->children[1], env); // Then block
    }
    else if (node->child_count > 2) {
        return eval_ast_node(node->children[2], env); // Else block (if present)
    }

    return make_null_value(); // No else block
}




/***********************************************************
* Function: eval_while_statement
* Description: this function evaluates the while statement.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_while_statement(ASTNode* node, RuntimeEnvironment* env) {
    if (node->child_count < 2) {
        return make_null_value();
    }

    ASTNode* conditionNode = node->children[0];
    ASTNode* bodyNode = node->children[1];

    while (1 && !env->function_returned) {
        // Evaluate condition
        RuntimeValue condVal = eval_ast_node(conditionNode, env);

        bool isTrue = false;
        if (condVal.type == RUNTIME_VALUE_BOOL) {
            isTrue = condVal.bool_val;
        }
        else if (condVal.type == RUNTIME_VALUE_INT) {
            isTrue = (condVal.int_val != 0);
        }

        if (!isTrue) {
            break;
        }

        // Evaluate the body
        RuntimeValue result = eval_ast_node(bodyNode, env);

        // Check for break signal
        if (result.type == RUNTIME_VALUE_SPECIAL && strcmp(result.special_val, "stop") == 0) {
            break;
        }
    }

    return make_null_value();
}




/***********************************************************
* Function: eval_for_statement
* Description: this function evaluates the for statement.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_for_statement(ASTNode* node, RuntimeEnvironment* env) {
    if (node->child_count < 3 && !env->function_returned) {
        return make_null_value();
    }

    // Evaluate start and end values once
    long start = eval_ast_node(node->children[0], env).int_val;
    long end = eval_ast_node(node->children[1], env).int_val;

    // Ensure body is ready
    ASTNode* bodyNode = node->children[2];

    // Loop execution
    for (long i = start; i < end; i++) {
        RuntimeValue indexValue = make_int_value(i);
        RuntimeValue result = eval_ast_node(bodyNode, env);
        // Check for break signal
        if (result.type == RUNTIME_VALUE_SPECIAL && strcmp(result.special_val, "stop") == 0) {
            break;
        }
    }

    return make_null_value();
}





/***********************************************************
* Function: eval_binary_expr
* Description: this function evaluates the binary expression.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_binary_expr(ASTNode* node, RuntimeEnvironment* env) {
    if (node->child_count < 2) {
        return make_null_value();
    }
    ASTNode* leftNode = node->children[0];
    ASTNode* rightNode = node->children[1];

    RuntimeValue leftVal = eval_ast_node(leftNode, env);
    RuntimeValue rightVal = eval_ast_node(rightNode, env);

    const char* op = node->operator_;

    if (strcmp(op, "+") == 0) {
        // If both int => int addition
        if (leftVal.type == RUNTIME_VALUE_INT && rightVal.type == RUNTIME_VALUE_INT) {
            return make_int_value(leftVal.int_val + rightVal.int_val);
        }
        // If both float => float addition
        if (leftVal.type == RUNTIME_VALUE_FLOAT && rightVal.type == RUNTIME_VALUE_FLOAT) {
            return make_float_value(leftVal.float_val + rightVal.float_val);
        }
        return make_null_value();
    }
    else if (strcmp(op, "-") == 0) {
        // Similar to above
        if (leftVal.type == RUNTIME_VALUE_INT && rightVal.type == RUNTIME_VALUE_INT) {
            return make_int_value(leftVal.int_val - rightVal.int_val);
        }
        if (leftVal.type == RUNTIME_VALUE_FLOAT && rightVal.type == RUNTIME_VALUE_FLOAT) {
            return make_float_value(leftVal.float_val - rightVal.float_val);
        }
        return make_null_value();
    }
    else if (strcmp(op, "*") == 0) {
        // ...
        if (leftVal.type == RUNTIME_VALUE_INT && rightVal.type == RUNTIME_VALUE_INT) {
            return make_int_value(leftVal.int_val * rightVal.int_val);
        }
        if (leftVal.type == RUNTIME_VALUE_FLOAT && rightVal.type == RUNTIME_VALUE_FLOAT) {
            return make_float_value(leftVal.float_val * rightVal.float_val);
        }
        return make_null_value();
    }
    else if (strcmp(op, "/") == 0) {
        // ...
        if (leftVal.type == RUNTIME_VALUE_INT && rightVal.type == RUNTIME_VALUE_INT) {
            if (rightVal.int_val == 0) {
                fprintf(stderr, "Runtime Error: division by zero.\n");
                return make_null_value();
            }
            return make_int_value(leftVal.int_val / rightVal.int_val);
        }
        if (leftVal.type == RUNTIME_VALUE_FLOAT && rightVal.type == RUNTIME_VALUE_FLOAT) {
            if (rightVal.float_val == 0.0) {
                fprintf(stderr, "Runtime Error: division by zero.\n");
                return make_null_value();
            }
            return make_float_value(leftVal.float_val / rightVal.float_val);
        }
        return make_null_value();
    }

    else if (strcmp(op, "%") == 0) {
        // ...
        if (leftVal.type == RUNTIME_VALUE_INT && rightVal.type == RUNTIME_VALUE_INT) {
            if (rightVal.int_val == 0) {
                fprintf(stderr, "Runtime Error: modulo by zero.\n");
                return make_null_value();
            }
            return make_int_value(leftVal.int_val % rightVal.int_val);
        }
        return make_null_value();
    }

    else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
        strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 || strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
        return evaluate_comparison(op, leftVal, rightVal);
    }

    return make_null_value();
}




/***********************************************************
* Function: evaluate_comparison
* Description: this function evaluates the comparison.
* Parameters: const char* op, RuntimeValue leftVal, RuntimeValue rightVal
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue evaluate_comparison(const char* op, RuntimeValue leftVal, RuntimeValue rightVal) {
    // Ensure both operands are of the same type, or convert if possible.
    if (leftVal.type != rightVal.type) {
        // Handle type mismatches (e.g., implicit conversions)
        // For simplicity, return false here. Extend this to handle type coercion.
        return make_bool_value(false);
    }

    if (strcmp(op, "&&") == 0) {
        bool left = (leftVal.type == RUNTIME_VALUE_BOOL && leftVal.bool_val);
        bool right = (rightVal.type == RUNTIME_VALUE_BOOL && rightVal.bool_val);
        return make_bool_value(left && right);
    }
    else if (strcmp(op, "||") == 0) {
        bool left = (leftVal.type == RUNTIME_VALUE_BOOL && leftVal.bool_val);
        bool right = (rightVal.type == RUNTIME_VALUE_BOOL && rightVal.bool_val);
        return make_bool_value(left || right);
    }

    switch (leftVal.type) {
    case RUNTIME_VALUE_INT: {
        long left = leftVal.int_val;
        long right = rightVal.int_val;

        if (strcmp(op, "==") == 0) return make_bool_value(left == right);
        if (strcmp(op, "!=") == 0) return make_bool_value(left != right);
        if (strcmp(op, "<") == 0) return make_bool_value(left < right);
        if (strcmp(op, ">") == 0) return make_bool_value(left > right);
        if (strcmp(op, "<=") == 0) return make_bool_value(left <= right);
        if (strcmp(op, ">=") == 0) return make_bool_value(left >= right);

        break;
    }

    case RUNTIME_VALUE_FLOAT: {
        double left = leftVal.float_val;
        double right = rightVal.float_val;

        if (strcmp(op, "==") == 0) return make_bool_value(left == right);
        if (strcmp(op, "!=") == 0) return make_bool_value(left != right);
        if (strcmp(op, "<") == 0) return make_bool_value(left < right);
        if (strcmp(op, ">") == 0) return make_bool_value(left > right);
        if (strcmp(op, "<=") == 0) return make_bool_value(left <= right);
        if (strcmp(op, ">=") == 0) return make_bool_value(left >= right);

        break;
    }

    case RUNTIME_VALUE_BOOL: {
        bool left = leftVal.bool_val;
        bool right = rightVal.bool_val;

        if (strcmp(op, "==") == 0) return make_bool_value(left == right);
        if (strcmp(op, "!=") == 0) return make_bool_value(left != right);

        // For booleans, <, >, <=, >= are typically not meaningful.
        break;
    }

    case RUNTIME_VALUE_STRING: {
        const char* left = leftVal.string_val;
        const char* right = rightVal.string_val;

        if (strcmp(op, "==") == 0) return make_bool_value(strcmp(left, right) == 0);
        if (strcmp(op, "!=") == 0) return make_bool_value(strcmp(left, right) != 0);
        if (strcmp(op, "<") == 0) return make_bool_value(strcmp(left, right) < 0);
        if (strcmp(op, ">") == 0) return make_bool_value(strcmp(left, right) > 0);
        if (strcmp(op, "<=") == 0) return make_bool_value(strcmp(left, right) <= 0);
        if (strcmp(op, ">=") == 0) return make_bool_value(strcmp(left, right) >= 0);

        break;
    }

    default:
        break;
    }

    // If no valid comparison was found, return false.
    return make_bool_value(false);
}




/***********************************************************
* Function: eval_function_call
* Description: this function evaluates the function call.
* Parameters: ASTNode* node, RuntimeEnvironment* env
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue eval_unary_expr(ASTNode* node, RuntimeEnvironment* env) {
    if (node->child_count < 1) {
        return make_null_value();
    }
    RuntimeValue val = eval_ast_node(node->children[0], env);
    const char* op = node->operator_;

    if (strcmp(op, "!") == 0) {
        // interpret val as bool
        bool isTrue = false;
        if (val.type == RUNTIME_VALUE_BOOL) {
            isTrue = val.bool_val;
        }
        else if (val.type == RUNTIME_VALUE_INT) {
            isTrue = (val.int_val != 0);
        }
        return make_bool_value(!isTrue);
    }
    else if (strcmp(op, "-") == 0) {
        // unary minus
        if (val.type == RUNTIME_VALUE_INT) {
            return make_int_value(-val.int_val);
        }
        if (val.type == RUNTIME_VALUE_FLOAT) {
            return make_float_value(-val.float_val);
        }
        // fallback
        return make_null_value();
    }
    else if (strcmp(op, "~") == 0) {
        // bitwise complement (only for int)
        if (val.type == RUNTIME_VALUE_INT) {
            return make_int_value(~val.int_val);
        }
        return make_null_value();
    }
    // etc.
    return make_null_value();
}
