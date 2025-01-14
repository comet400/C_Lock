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

    case AST_ASSIGNMENT:
        generate_assignment_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_BINARY_EXPR:
        generate_binary_expr_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_LITERAL:
        generate_literal_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;

    case AST_IDENTIFIER:
        generate_identifier_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
        break;
    
	case AST_UNARY_EXPR:
		generate_unary_exp_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
		break;

	case AST_IF_STATEMENT:
		generate_if_statement_bytecode(node, bytecode, bytecode_count, bytecode_capacity);
		break;
	case AST_BLOCK:
		for (size_t i = 0; i < node->child_count; i++) {
			generate_bytecode(node->children[i], bytecode, bytecode_count, bytecode_capacity);
		}
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
