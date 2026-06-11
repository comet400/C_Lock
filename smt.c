#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "runtimeEnv.h"
#include "runtimeValue.h"

int main(void) {
    RuntimeEnvironment* env = create_environment(NULL);
    built_in_functions(env);
    RuntimeValue __t0 = make_null_value();
    { /* call timer_start */
        RuntimeValue __t1 = env_get_func(env, "timer_start");
        if (__t1.type != RUNTIME_VALUE_BUILTIN) { fprintf(stderr, "AOT runtime: function not callable: timer_start\n"); __t0 = make_null_value(); } else {
            RuntimeValue __args[1];
            __t0 = __t1.builtin_val.fn(__args, 0);
        }
    } /* end call timer_start */
    RuntimeValue __t2 = make_int_value(0);
    env_set_var(env, "counter", __t2);
    RuntimeValue __t3 = make_int_value(0);
    RuntimeValue __t4 = make_int_value(300000000);
    for (long __t5 = __t3.int_val; __t5 < __t4.int_val; __t5++) {
        RuntimeValue __t6 = make_int_value(1);
        RuntimeValue __t7 = env_get_var(env, "counter");
        RuntimeValue __newv = make_null_value();
        __newv = make_int_value(__t7.int_val + __t6.int_val);
        env_set_var(env, "counter", __newv);
    }
    RuntimeValue __t8 = make_null_value();
    { /* call timer_elapsed_ms */
        RuntimeValue __t9 = env_get_func(env, "timer_elapsed_ms");
        if (__t9.type != RUNTIME_VALUE_BUILTIN) { fprintf(stderr, "AOT runtime: function not callable: timer_elapsed_ms\n"); __t8 = make_null_value(); } else {
            RuntimeValue __args[1];
            __t8 = __t9.builtin_val.fn(__args, 0);
        }
    } /* end call timer_elapsed_ms */
    env_set_var(env, "ms", __t8);
    RuntimeValue __t10 = make_null_value();
    { /* call write */
        RuntimeValue __t11 = env_get_func(env, "write");
        if (__t11.type != RUNTIME_VALUE_BUILTIN) { fprintf(stderr, "AOT runtime: function not callable: write\n"); __t10 = make_null_value(); } else {
            RuntimeValue __args[1];
            RuntimeValue __t12 = make_string_value("Loop count: ");
            __args[0] = __t12;
            __t10 = __t11.builtin_val.fn(__args, 1);
        }
    } /* end call write */
    RuntimeValue __t13 = make_null_value();
    { /* call write */
        RuntimeValue __t14 = env_get_func(env, "write");
        if (__t14.type != RUNTIME_VALUE_BUILTIN) { fprintf(stderr, "AOT runtime: function not callable: write\n"); __t13 = make_null_value(); } else {
            RuntimeValue __args[1];
            RuntimeValue __t15 = env_get_var(env, "counter");
            if (__t15.type == RUNTIME_VALUE_NULL) __t15 = env_get_func(env, "counter");
            __args[0] = __t15;
            __t13 = __t14.builtin_val.fn(__args, 1);
        }
    } /* end call write */
    RuntimeValue __t16 = make_null_value();
    { /* call write */
        RuntimeValue __t17 = env_get_func(env, "write");
        if (__t17.type != RUNTIME_VALUE_BUILTIN) { fprintf(stderr, "AOT runtime: function not callable: write\n"); __t16 = make_null_value(); } else {
            RuntimeValue __args[1];
            RuntimeValue __t18 = make_string_value("Time (ms): ");
            __args[0] = __t18;
            __t16 = __t17.builtin_val.fn(__args, 1);
        }
    } /* end call write */
    RuntimeValue __t19 = make_null_value();
    { /* call write */
        RuntimeValue __t20 = env_get_func(env, "write");
        if (__t20.type != RUNTIME_VALUE_BUILTIN) { fprintf(stderr, "AOT runtime: function not callable: write\n"); __t19 = make_null_value(); } else {
            RuntimeValue __args[1];
            RuntimeValue __t21 = env_get_var(env, "ms");
            if (__t21.type == RUNTIME_VALUE_NULL) __t21 = env_get_func(env, "ms");
            __args[0] = __t21;
            __t19 = __t20.builtin_val.fn(__args, 1);
        }
    } /* end call write */
    free_environment(env);
    return 0;
}
