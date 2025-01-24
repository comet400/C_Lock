/***********************************************************
* File: bytecode.C
* This file have the bytecode for the interpreter.
* The bytecode is used to store the instructions for the machine compiler (machine type: 86x64).
* This Code was written by Lukas Fukuoka Vieira.
* Contact: lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/




#include "bytecode.h"

const char* ByteCodeNames[] = {
    "OP_PUSH_INT",
    "OP_PUSH_FLOAT",
    "OP_PUSH_BOOL",
    "OP_PUSH_STRING",
    "OP_ADD_",
    "OP_SUBTRACT",
    "OP_MULTIPLY",
    "OP_DIVIDE",
    "OP_ASSIGN",
    "OP_JUMP_",
    "OP_JUMP_TO_IF_FALSE",
    "OP_CALL_FUNCTION",
    "OP_DECL_FUNCTION",
    "OP_RETURN_",
    "OP_NEGATE",
    "OP_MODULO",
    "OP_MODULO_EQUAL",  
    "OP_LESS",
    "OP_GREATER",
    "OP_LESS_EQUAL",
    "OP_GREATER_EQUAL",
    "OP_EQUAL",         
    "OP_NOT_EQUAL",
    "OP_AND_",
    "OP_OR_",
    "OP_NOT_",
    "OP_PRINT_",
    "OP_INPUT_",
    "OP_ARRAY_SET_",
    "OP_ARRAY_GET_",
    "OP_LOAD_VAR_",
    "OP_STORE_VAR_",
    "OP_LOAD_CONST_",
    "OP_POP",
    "OP_HALT",
    "OP_JUMP_IF_TRUE_",
    "OP_JUMP_IF_FALSE_",
    "OP_JUMP_TO",
    "OP_CALL_",
    "OP_RETURN_STM",
    "OP_BREAK_",
    "OP_CONTINUE_",
    "OP_SWITCH_",
    "OP_WHEN_",
    "OP_DEFAULT_",
    "OP_PARAMETER_LIST_",
    "OP_SCOPE"
};




/***********************************************************
 * Function: ensure_bytecode_capacity
 * Description: this function is used to ensure that the bytecode has enough capacity for the new instruction.
 * Parameters: BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void ensure_bytecode_capacity(BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (*bytecode_count >= *bytecode_capacity) {
        *bytecode_capacity *= 2;
        *bytecode = realloc(*bytecode, sizeof(BytecodeInstruction) * (*bytecode_capacity));
        if (!*bytecode) {
            fprintf(stderr, "Memory allocation failed during bytecode generation.\n");
            exit(EXIT_FAILURE);
        }
    }
}






/***********************************************************
 * Function: generate_literal_bytecode
 * Description: this function generates bytecode for a literal value.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_literal_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);

    BytecodeInstruction instr = { .opcode = OP_PUSH_INT };
    if (node->value_kind == VALUE_INT) {
        instr.operand.int_operand = node->value.int_val;
    }
    else if (node->value_kind == VALUE_FLOAT) {
        instr.opcode = OP_PUSH_FLOAT;
        instr.operand.float_operand = node->value.float_val;
    }
	else if (node->value_kind == VALUE_BOOL) {
		instr.opcode = OP_PUSH_BOOL;
		instr.operand.bool_operand = node->value.bool_val;
	}
	else if (node->value_kind == VALUE_STRING) {
		instr.opcode = OP_PUSH_STRING;
		instr.operand.string_operand = node->value.str_val;
	}
    else {
        fprintf(stderr, "Unsupported literal type in AST_LITERAL\n");
        exit(EXIT_FAILURE);
    }

    (*bytecode)[(*bytecode_count)++] = instr;
}






/***********************************************************
 * Function: generate_binary_expr_bytecode
 * Description: this function generates bytecode for a binary expression.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_binary_expr_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    // Recursively generate bytecode for left and right children
    BytecodeInstruction instr;

    generate_bytecode(node->children[0], bytecode, bytecode_count, bytecode_capacity);
	instr.operand.binary.left_reg = *bytecode_count - 1;
    generate_bytecode(node->children[1], bytecode, bytecode_count, bytecode_capacity);
	instr.operand.binary.right_reg = *bytecode_count - 1;

    // Generate bytecode for the binary operation
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);

    if (strcmp(node->operator_, "+") == 0) {
        instr.opcode = OP_ADD_;
    }
    else if (strcmp(node->operator_, "-") == 0) {
        instr.opcode = OP_SUBTRACT;
    }
    else if (strcmp(node->operator_, "*") == 0) {
        instr.opcode = OP_MULTIPLY;
    }
    else if (strcmp(node->operator_, "/") == 0) {
        instr.opcode = OP_DIVIDE;
    }
    else if (strcmp(node->operator_, "%") == 0) {
        instr.opcode = OP_MODULO;
    }
    else if (strcmp(node->operator_, "<") == 0) {
        instr.opcode = OP_LESS;
    }
    else if (strcmp(node->operator_, ">") == 0) {
        instr.opcode = OP_GREATER;
    }
    else if (strcmp(node->operator_, "<=") == 0) {
        instr.opcode = OP_LESS_EQUAL;
    }
    else if (strcmp(node->operator_, ">=") == 0) {
        instr.opcode = OP_GREATER_EQUAL;
    }
    else if (strcmp(node->operator_, "==") == 0) {
        instr.opcode = OP_EQUAL;
    }
    else if (strcmp(node->operator_, "!=") == 0) {
        instr.opcode = OP_NEGATE;
    }
	else if (strcmp(node->operator_, "%=") == 0) {
		instr.opcode = OP_MODULO_EQUAL;
	}
    else if (strcmp(node->operator_, "&&") == 0) {
        instr.opcode = OP_AND_;
    }
	else if (strcmp(node->operator_, "||") == 0) {
		instr.opcode = OP_OR_;
	}
	else if (strcmp(node->operator_, ",") == 0) {
		return;
	}
    else {
        fprintf(stderr, "Unsupported binary operator: %s\n", node->operator_);
        exit(EXIT_FAILURE);
    }

    (*bytecode)[(*bytecode_count)++] = instr;
}










/***********************************************************
 * Function: generate_assignment_bytecode
 * Description: this function generates bytecode for an assignment.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_assignment_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    // Generate bytecode for the right-hand side (RHS)
    generate_bytecode(node->children[1], bytecode, bytecode_count, bytecode_capacity);

    // Generate a STORE instruction for the left-hand side (LHS)
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);
    BytecodeInstruction instr = {
        .opcode = OP_STORE_VAR_,
        .operand.string_operand = node->children[0]->operator_
    };
    (*bytecode)[(*bytecode_count)++] = instr;
}





/***********************************************************
 * Function: generate_identifier_bytecode
 * Description: this function generates bytecode for an identifier.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_identifier_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);
    BytecodeInstruction instr = {
        .opcode = OP_LOAD_VAR_,
        .operand.string_operand = node->operator_
    };
    (*bytecode)[(*bytecode_count)++] = instr;
}




/***********************************************************
 * Function: generate_for_bytecode
 * Description: this function is responsible for generating bytecode for a 'for' loop.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_for_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (!node || node->child_count != 3) {
        fprintf(stderr, "Invalid AST structure for 'for' loop.\n");
        exit(EXIT_FAILURE);
    }

    // Extract components: start, end, and body
    const ASTNode* start_node = node->children[0];  // Start value
    const ASTNode* end_node = node->children[1];    // End value
    const ASTNode* body_node = node->children[2];   // Loop body

    const char* loop_var_name = "i"; // Loop variable name (default for simplicity)

    // 1. Initialization: i = start
    generate_bytecode(start_node, bytecode, bytecode_count, bytecode_capacity);
    BytecodeInstruction init_instr = {
        .opcode = OP_STORE_VAR_,
        .operand.string_operand = loop_var_name
    };
    (*bytecode)[(*bytecode_count)++] = init_instr;

    // 2. Condition check: i < end
    size_t condition_index = *bytecode_count;
    BytecodeInstruction load_var_instr = {
        .opcode = OP_LOAD_VAR_,
        .operand.string_operand = loop_var_name
    };
    (*bytecode)[(*bytecode_count)++] = load_var_instr;

    generate_bytecode(end_node, bytecode, bytecode_count, bytecode_capacity);

    BytecodeInstruction condition_instr = { .opcode = OP_LESS };
	condition_instr.operand.binary.left_reg = *bytecode_count - 2;
	condition_instr.operand.binary.right_reg = *bytecode_count - 1;
    (*bytecode)[(*bytecode_count)++] = condition_instr;

    // Placeholder for JUMP_TO_IF_FALSE
    BytecodeInstruction jump_if_false_instr = {
        .opcode = OP_JUMP_TO_IF_FALSE,
        .operand.int_operand = -1 // Placeholder
    };
    size_t jump_if_false_index = *bytecode_count;
    (*bytecode)[(*bytecode_count)++] = jump_if_false_instr;

    // 3. Loop body
    generate_bytecode(body_node, bytecode, bytecode_count, bytecode_capacity);

    // 4. Increment: i = i + 1
    BytecodeInstruction load_loop_var_instr = {
        .opcode = OP_LOAD_VAR_,
        .operand.string_operand = loop_var_name
    };
    (*bytecode)[(*bytecode_count)++] = load_loop_var_instr;

    BytecodeInstruction push_constant_instr = {
        .opcode = OP_PUSH_INT,
        .operand.int_operand = 1 // Increment
    };
    (*bytecode)[(*bytecode_count)++] = push_constant_instr;

    BytecodeInstruction increment_instr = { .opcode = OP_ADD_ };
	increment_instr.operand.binary.left_reg = *bytecode_count - 2;
	increment_instr.operand.binary.right_reg = *bytecode_count - 1;
    (*bytecode)[(*bytecode_count)++] = increment_instr;

    BytecodeInstruction store_loop_var_instr = {
        .opcode = OP_STORE_VAR_,
        .operand.string_operand = loop_var_name
    };
    (*bytecode)[(*bytecode_count)++] = store_loop_var_instr;

    // 5. Jump back to condition check
    BytecodeInstruction jump_to_condition_instr = {
        .opcode = OP_JUMP_TO,
        .operand.int_operand = condition_index
    };
    (*bytecode)[(*bytecode_count)++] = jump_to_condition_instr;

    // Update the JUMP_TO_IF_FALSE placeholder
    (*bytecode)[jump_if_false_index].operand.int_operand = *bytecode_count;
}


	


/***********************************************************
 * Function: generate_bytecode
 * Description: this function generates bytecode for an AST.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (!node) return;

    switch (node->type) {
    case AST_PROGRAM:
        for (size_t i = 0; i < node->child_count; i++) {
            generate_bytecode(node->children[i], bytecode, bytecode_count, bytecode_capacity);
        }
        break;

    case AST_BLOCK:
        for (size_t i = 0; i < node->child_count; i++) {
            generate_bytecode(node->children[i], bytecode, bytecode_count, bytecode_capacity);
        }
        break;

    case AST_LITERAL:
        generate_literal_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_ASSIGNMENT:
        generate_assignment_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_IDENTIFIER:
        generate_identifier_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_IF_STATEMENT:
        generate_if_statement_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_WHILE_STATEMENT:
        generate_while_loop_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_FOR_STATEMENT:
        generate_for_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_BINARY_EXPR:
        generate_binary_expr_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;
    
	case AST_UNARY_EXPR:
		generate_unary_exp_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
		break;
	case AST_FUNCTION_DECLARATION:
		generate_function_declaration_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
		break;
	case AST_FUNCTION_CALL:
		generate_function_call_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
		break;

    case AST_RETURN_STATEMENT:
		generate_return_statement_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
		break;

	case AST_ARRAY_LITERAL:
		generate_array_literal_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
		break;

    default:
        fprintf(stderr, "Unhandled ASTNode type: %d\n", node->type);
        exit(EXIT_FAILURE);
    }
}




/***********************************************************
 * Function: generate_unary_exp_bytecode
 * Description: this function generates bytecode for a unary expression.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_unary_exp_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    // Recursively generate bytecode for the operand
    generate_bytecode(node->children[0], bytecode, bytecode_count, bytecode_capacity);

    // Generate bytecode for the unary operation
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);

    BytecodeInstruction instr;
    if (strcmp(node->operator_, "-") == 0) {
        instr.opcode = OP_NEGATE;
    }
    else if (strcmp(node->operator_, "!") == 0) {
        instr.opcode = OP_NOT_;
    }
    else {
        fprintf(stderr, "Unsupported unary operator: %s\n", node->operator_);
        exit(EXIT_FAILURE);
    }

    (*bytecode)[(*bytecode_count)++] = instr;
}





/***********************************************************
 * Function: generate_if_statement_bytecode
 * Description: this function generates bytecode for an if statement.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 * ***********************************************************/
void generate_if_statement_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    // Generate bytecode for the condition
    generate_bytecode(node->children[0], bytecode, bytecode_count, bytecode_capacity);

    // Generate a JUMP_TO_IF_FALSE instruction with a placeholder index
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);
    BytecodeInstruction jumpToFalseInstr = {
        .opcode = OP_JUMP_TO_IF_FALSE,
        .operand.int_operand = -1 // Placeholder
    };
    size_t jumpToFalseIndex = *bytecode_count; // Save the index for later update
    (*bytecode)[(*bytecode_count)++] = jumpToFalseInstr;

    // Generate bytecode for the body of the if statement
    generate_bytecode(node->children[1], bytecode, bytecode_count, bytecode_capacity);

    // If there is an else block, generate a JUMP instruction to skip the else body
    size_t jumpToEndIndex = -1; // Placeholder index for the unconditional jump
    if (node->child_count > 2) {
        ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);
        BytecodeInstruction jumpToEndInstr = {
            .opcode = OP_JUMP_TO,
            .operand.int_operand = -1 // Placeholder
        };
        jumpToEndIndex = *bytecode_count; // Save the index for later update
        (*bytecode)[(*bytecode_count)++] = jumpToEndInstr;
    }

    // Update the JUMP_TO_IF_FALSE to point to the first instruction after the "if" body
    (*bytecode)[jumpToFalseIndex].operand.int_operand = *bytecode_count;

    // Generate bytecode for the else block (if it exists)
    if (node->child_count > 2) {
        generate_bytecode(node->children[2], bytecode, bytecode_count, bytecode_capacity);

        // Update the unconditional jump to point to the end of the else body
        (*bytecode)[jumpToEndIndex].operand.int_operand = *bytecode_count;
    }
}





/***********************************************************
 * Function: generate_while_loop_bytecode
 * Description: This function generates bytecode for a while loop.
 * Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
 * Return: void
 ***********************************************************/
void generate_while_loop_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (!node || node->child_count != 2) {
        fprintf(stderr, "Invalid AST structure for 'while' loop.\n");
        exit(EXIT_FAILURE);
    }

    // Save the index of the condition check
    size_t conditionIndex = *bytecode_count;

    // Generate bytecode for the condition
    generate_bytecode(node->children[0], bytecode, bytecode_count, bytecode_capacity);

    // Generate a JUMP_TO_IF_FALSE instruction with a placeholder index
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);
    BytecodeInstruction jumpToFalseInstr = {
        .opcode = OP_JUMP_TO_IF_FALSE,
        .operand.int_operand = -1 // Placeholder
    };
    size_t jumpToFalseIndex = *bytecode_count; // Save the index for later update
    (*bytecode)[(*bytecode_count)++] = jumpToFalseInstr;

    // Generate bytecode for the body of the while loop
    generate_bytecode(node->children[1], bytecode, bytecode_count, bytecode_capacity);

    // Generate a JUMP_TO instruction to loop back to the condition
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);
    BytecodeInstruction jumpToConditionInstr = {
        .opcode = OP_JUMP_TO,
        .operand.int_operand = conditionIndex
    };
    (*bytecode)[(*bytecode_count)++] = jumpToConditionInstr;

    // Update the JUMP_TO_IF_FALSE to point to the first instruction after the loop
    (*bytecode)[jumpToFalseIndex].operand.int_operand = *bytecode_count;
}





/***********************************************************
 * Function: print_byteCode
 * Description: this function prints the bytecode.
 * Parameters: const BytecodeInstruction* bytecode, size_t count
 * Return: void
 * ***********************************************************/
void print_byteCode(const BytecodeInstruction* bytecode, size_t count) {
    printf("=== BYTECODE ===\n");
    for (size_t i = 0; i < count; i++) {
        const BytecodeInstruction* instr = &bytecode[i];
		if (instr->opcode >= sizeof(ByteCodeNames) / sizeof(ByteCodeNames[0])) {
			fprintf(stderr, "Invalid opcode: %d\n", instr->opcode);
			exit(EXIT_FAILURE);
		}

        printf("[%3zu] %s", i, ByteCodeNames[instr->opcode]); // Print opcode name

        switch (instr->opcode) {
        case OP_PUSH_INT:
            printf(" INT_OPERAND: %d\n", instr->operand.int_operand);
            break;

        case OP_PUSH_FLOAT:
            printf(" FLOAT_OPERAND: %f\n", instr->operand.float_operand);
            break;

        case OP_PUSH_BOOL:
            printf(" BOOL_OPERAND: %s\n", instr->operand.bool_operand ? "true" : "false");
            break;

        case OP_PUSH_STRING:
            printf(" STRING_OPERAND: \"%s\"\n", instr->operand.string_operand);
            break;

        case OP_LOAD_VAR_:
        case OP_STORE_VAR_:
            printf(" VAR_NAME: \"%s\"\n", instr->operand.string_operand);
            break;

        case OP_JUMP_TO_IF_FALSE:
        case OP_JUMP_TO:
            printf(" TARGET_INDEX: %d\n", instr->operand.jump.target_index);
            break;

        case OP_ADD_:
        case OP_SUBTRACT:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_MODULO:
        case OP_EQUAL:
        case OP_GREATER:
        case OP_LESS:
            printf(" BINARY_OP: (LEFT_REG: %d, RIGHT_REG: %d)\n",
                instr->operand.binary.left_reg,
                instr->operand.binary.right_reg);
            break;

        case OP_NEGATE:
        case OP_NOT_:
            printf(" UNARY_OP: (REG: %d)\n", instr->operand.unary.reg);
            break;

        case OP_ARRAY_GET_:
            printf(" ARRAY_ACCESS: (ARRAY_REG: %d, INDEX_REG: %d)\n",
                instr->operand.array_access.array_reg,
                instr->operand.array_access.index_reg);
            break;

		case OP_ARRAY_SET_:
            printf(" ARRAY_LITERAL: (ARRAY_REG: %d, COUNT: %d)\n",
                instr->operand.array_literal.array_reg,
                instr->operand.array_literal.count);
            break;

		case OP_DECL_FUNCTION:
            printf(" FUNCTION_DECL: (PARAM_COUNT: %d, BODY_INDEX: %d)\n",
                instr->operand.function_decl.param_count,
                instr->operand.function_decl.body_index);
            break;

        case OP_SWITCH_:
            printf(" SWITCH: (DEFAULT_INDEX: %d, WHEN_COUNT: %d)\n",
                instr->operand.switch_.default_index,
                instr->operand.switch_.when_count);
            break;

        case OP_WHEN_:
            printf(" WHEN: (CONDITION_REG: %d, BODY_INDEX: %d)\n",
                instr->operand.when.condition_reg,
                instr->operand.when.body_index);
            break;

        case OP_PARAMETER_LIST_:
            printf(" PARAM_LIST: (PARAM_COUNT: %d)\n", instr->operand.param_list.param_count);
            break;

        case OP_SCOPE:
            printf(" SCOPE: (LOCAL_INDEX: %d, GLOBAL_INDEX: %d)\n",
                instr->operand.scope.local_index,
                instr->operand.scope.global_index);
            break;

        default:
            printf("\n");
            break;
        }
    }
    printf("=== END BYTECODE ===\n");
}






/***********************************************************
* Function: generate_function_declaration_bytecode
* Description: Generates bytecode for function declarations.
* Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
* Return: void
***********************************************************/
void generate_function_declaration_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (!node || node->type != AST_FUNCTION_DECLARATION) {
        fprintf(stderr, "Invalid node type for function declaration.\n");
        exit(EXIT_FAILURE);
    }

    // Ensure bytecode capacity
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);

    // The first child is the function name (identifier node)
    const ASTNode* identifierNode = node->children[0];
    if (!identifierNode || identifierNode->type != AST_IDENTIFIER || !identifierNode->isFunction) {
        fprintf(stderr, "Invalid function identifier.\n");
        exit(EXIT_FAILURE);
    }

    // Add function declaration instruction
    BytecodeInstruction instr = {
        .opcode = OP_DECL_FUNCTION,
        .operand.function_decl.param_count = node->child_count - 2, // Remaining children after identifier and body
        .operand.function_decl.body_index = *bytecode_count + 1    // Body starts next
    };
    (*bytecode)[(*bytecode_count)++] = instr;

    // Track whether a return statement has been encountered
    bool has_returned = false;

    // Generate bytecode for the function body (last child is the body block)
    const ASTNode* body = node->children[node->child_count - 1];
    for (size_t i = 0; i < body->child_count; i++) {
        if (has_returned) {
            // Skip generating bytecode for statements after a return
            break;
        }

        const ASTNode* statement = body->children[i];

        // Check if the statement is a return
        if (statement->type == AST_RETURN_STATEMENT) {
            has_returned = true;
        }

        // Generate bytecode for the statement
        generate_bytecode(statement, bytecode, bytecode_count, bytecode_capacity);
    }

    // Add return instruction if there wasn't an explicit return in the function
    if (!has_returned) {
        ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);
        instr.opcode = OP_RETURN_;
        (*bytecode)[(*bytecode_count)++] = instr;
    }
}





/***********************************************************
* Function: generate_function_call_bytecode
* Description: Generates bytecode for function calls.
* Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
* Return: void
***********************************************************/
void generate_function_call_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (!node || node->type != AST_FUNCTION_CALL) {
        fprintf(stderr, "Invalid node type for function call.\n");
        exit(EXIT_FAILURE);
    }

    // The first child is the function name (identifier node)
    const ASTNode* identifierNode = node->children[0];
    if (!identifierNode || identifierNode->type != AST_IDENTIFIER || !identifierNode->isFunction) {
        fprintf(stderr, "Invalid function identifier in call.\n");
        exit(EXIT_FAILURE);
    }

    // Generate bytecode for each argument (remaining children)
    for (size_t i = 1; i < node->child_count; i++) {
        generate_bytecode(node->children[i], bytecode, bytecode_count, bytecode_capacity);
    }

    // Ensure bytecode capacity
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);

    // Add function call instruction
    BytecodeInstruction instr = {
        .opcode = OP_CALL_FUNCTION,
        .operand.string_operand = identifierNode->operator_  // Function name
    };
    (*bytecode)[(*bytecode_count)++] = instr;

    // Handle the return value if necessary
}




/***********************************************************
* Function: traverse_binary_expression
* Description: Traverses a binary expression tree to generate bytecode for array literals.
* Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity, size_t* element_count
* Return: void
* **********************************************************/
void traverse_binary_expression(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity, size_t* element_count) {
    if (!node) {
        fprintf(stderr, "Invalid node in binary expression traversal.\n");
        exit(EXIT_FAILURE);
    }

    if (node->type == AST_BINARY_EXPR && strcmp(node->operator_, ",") == 0) {
        // Handle the binary expression for a comma operator
        // Recursively traverse the left child
        traverse_binary_expression(node->children[0], bytecode, bytecode_count, bytecode_capacity, element_count);

        // Increment the element count for the comma
        (*element_count)++;

        // Recursively traverse the right child
        traverse_binary_expression(node->children[1], bytecode, bytecode_count, bytecode_capacity, element_count);
    }
    else {
        // For other cases
        generate_bytecode(node, bytecode, bytecode_count, bytecode_capacity);

        // Only increment for the first item (element count starts at 1)
        if (*element_count == 0) {
            (*element_count) = 1;
        }
    }
}





/***********************************************************
* Function: generate_array_literal_bytecode
* Description: Generates bytecode for array literals.
* Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
* Return: void
* ***********************************************************/
void generate_array_literal_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (!node || node->type != AST_ARRAY_LITERAL) {
        fprintf(stderr, "Invalid node type for array literal.\n");
        exit(EXIT_FAILURE);
    }

    // Counter to track the number of elements pushed
    size_t element_count = 0;

    // Traverse the binary expression tree to generate bytecode for all elements
    traverse_binary_expression(node->children[0], bytecode, bytecode_count, bytecode_capacity, &element_count);

    // Ensure capacity for the array creation instruction
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);

    // Add an instruction to create an array with the specified number of elements
    BytecodeInstruction instr = {
        .opcode = OP_ARRAY_SET_, // Replace with the correct opcode for array creation
        .operand.array_literal.count = element_count // Use the tracked element count
    };
    (*bytecode)[(*bytecode_count)++] = instr;

    // Optionally, print or log the number of elements for debugging
    printf("Generated array literal bytecode with %zu elements.\n", element_count);
}





/***********************************************************
* Function: generate_return_statement_bytecode
* Description: Generates bytecode for a `return` statement.
* Parameters: const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity
* Return: void
***********************************************************/
void generate_return_statement_bytecode(const ASTNode* node, BytecodeInstruction** bytecode, size_t* bytecode_count, size_t* bytecode_capacity) {
    if (!node || node->type != AST_RETURN_STATEMENT) {
        fprintf(stderr, "Invalid node type for return statement.\n");
        exit(EXIT_FAILURE);
    }

    // Check if the return statement has a value
    if (node->child_count > 0) {
        // Generate bytecode for the return expression
        generate_bytecode(node->children[0], bytecode, bytecode_count, bytecode_capacity);
    }

    // Ensure capacity for the return instruction
    ensure_bytecode_capacity(bytecode, bytecode_count, bytecode_capacity);

    // Add the return instruction
    BytecodeInstruction instr = { .opcode = OP_RETURN_ };
    (*bytecode)[(*bytecode_count)++] = instr;
}

