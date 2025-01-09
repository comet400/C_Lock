/**********************************************
* File: runtimeValue.h
* This file allows the creation of runtime values for the interpreter.
* the runtime values are used to store the values of variables and functions on our environment.
* This Code was written by Lukas Fukuoka Vieira.
* Contact : lukas.fvieira@hotmail.com
* GitHub:https://github.com/comet400
************************************************************/



#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lexer.h"
#include "runtimeValue.h"

/***********************************************************
* Function: make_int_value
* Description: this function prepares a runtime value of type int.
* Parameters: long i
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue make_int_value(long i) {
    RuntimeValue v;
    v.type = RUNTIME_VALUE_INT;
    v.int_val = i;
    return v;
}




/***********************************************************
* Function: make_float_value
* Description: this function prepares a runtime value of type float.
* Parameters: double f
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue make_float_value(double f) {
    RuntimeValue v;
    v.type = RUNTIME_VALUE_FLOAT;
    v.float_val = f;
    return v;
}





/***********************************************************
* Function: make_bool_value
* Description: this function prepares a runtime value of type bool.
* Parameters: bool b
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue make_bool_value(bool b) {
    RuntimeValue v;
    v.type = RUNTIME_VALUE_BOOL;
    v.bool_val = b;
    return v;
}




/***********************************************************
* Function: make_string_value
* Description: this function prepares a runtime value of type string.
* Parameters: const char* s
* Return: RuntimeValue
* ***********************************************************/ 
RuntimeValue make_string_value(const char* s) {
    RuntimeValue v;
    v.type = RUNTIME_VALUE_STRING;
    if (s) {
        size_t len = strlen(s);
        v.string_val = (char*)malloc(len + 1);

		// Lets use strcpy_s for MSVC since it's safer
#ifdef _MSC_VER
        if (v.string_val == NULL) {
            fprintf(stderr, "Memory allocation failed in make_string_value\n");
            exit(EXIT_FAILURE);
        }

        strcpy_s(v.string_val, len + 1, s);
#else
        strcpy(v.string_val, s);
#endif
    }
    else {
        v.string_val = NULL;
    }
    return v;
}




/***********************************************************
* Function: make_array_value
* Description: this function prepares a runtime value of type array.
* Parameters: RuntimeValue* elements, size_t count
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue make_builtin_function(RuntimeValue(*fn)(RuntimeValue* args, size_t argc)) {
    RuntimeValue value;
    value.type = RUNTIME_VALUE_BUILTIN;
    value.builtin_val.fn = fn;
    return value;
}





/***********************************************************
* Function: make_array_value
* Description: this function prepares a runtime value of type array.
* Parameters: RuntimeValue* elements, size_t count
* Return: RuntimeValue
* ***********************************************************/
RuntimeValue make_null_value(void) {
    RuntimeValue v;
    v.type = RUNTIME_VALUE_NULL;
    return v;
}
