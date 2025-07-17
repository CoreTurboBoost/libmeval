
#include "meval/meval.h"
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define REPL_INPUT_BUFFER_LEN 128
#define MIN(a, b) (a < b ? a : b)

#ifdef MEVAL_DB_ENABLED
#define DBPRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DBPRINT(format, ...)
#endif

bool match_option(const char* option, char input_buffer[]) {
    size_t input_buf_len = strlen(input_buffer);
    if (input_buf_len < 1) {
        return false;
    }
    return strncmp(option, input_buffer, input_buf_len) == 0;
}

void get_input_line(const char* prompt, char input_buffer[], size_t *input_len) {
    /* Assume input_buffer is of length REPL_INPUT_BUFFER_LEN */
    printf("%s", prompt);
    char* input_return = fgets(input_buffer, REPL_INPUT_BUFFER_LEN, stdin);
    size_t len = strlen(input_buffer);
    if (len > 0 && input_buffer[len-1] == '\n') {
        input_buffer[len-1] = '\0';
        len--;
    }
    if (input_len != NULL) {
        *input_len = len;
    }
    if (strcmp("exit\n", input_buffer) == 0 || input_return == NULL) {
        printf("\r");
        exit(0);
    }
}

MEvalVarArr parse_for_vars(char input[], size_t input_len) {
    MEvalVarArr vars = {0};
    char* var_start = NULL;
    size_t var_char_len = 0;
    char* num_start = NULL;
    size_t num_char_len = 0;
    bool in_var_stage = true;
    for (size_t i = 0; i < input_len; i++) {
        if (isspace(input[i]) || i == input_len-1) {
            if (i == input_len-1) {
                if (num_start == NULL && !in_var_stage) {
                    num_start = &input[i];
                }
                if (!in_var_stage) {
                    num_char_len++;
                }
            }
            /*
            if (i == input_len-1) {
                if (in_var_stage) {
                    var_char_len++;
                } else {
                    num_char_len++;
                }
            }
            */
            if (var_start != NULL && num_start != NULL) {
                MEvalVar new_var = {0};
                new_var.name_char_count = num_char_len;
                DBPRINT("new_var.name_char_count: %d\n", new_var.name_char_count);
                char number_buf[256] = {0};
                snprintf(number_buf, MIN(255, num_char_len+1), "%s", num_start);
                DBPRINT("db: repl: number to be parsed: '%s'\n", number_buf);
                new_var.value = atof(number_buf);
                DBPRINT("db: repl: new_var.value = %f\n", new_var.value);
                snprintf(new_var.name, MIN(MEVAL_VAR_NAME_MAX_LEN, var_char_len+1), "%s", var_start);
                DBPRINT("db: repl: new_var.name = %s\n", new_var.name);
                bool success = meval_append_variable(&vars, new_var);
                if (success == false) {
                    DBPRINT("db: repl: Failed to allocate new_var to vars array\n");
                }
            }
            var_start = NULL;
            num_start = NULL;
            var_char_len = 0;
            num_char_len = 0;
            continue;
        } else if (input[i] == '=') {
            in_var_stage = false;
            continue;
        } 
        if (var_start == NULL) {
            var_start = &input[i];
            in_var_stage = true;
        } 
        if (num_start == NULL && !in_var_stage) {
            num_start = &input[i];
        }
        if (in_var_stage) {
            var_char_len++;
        } else {
            num_char_len++;
        }
    }
    return vars;
}

void print_vars(MEvalVarArr vars) {
    for (uint32_t i = 0; i<vars.elements_count; i++) {
        DBPRINT("Var[%d] '%s' = %f\n", i, vars.arr_ptr[i].name, vars.arr_ptr[i].value);
    }
}

void print_usage(const char* program_name) {
    printf("Usage: %s [expr]...\n", program_name);
}

int main(int argc, char* argv[]) {

    MEvalError error;

    uint32_t failure_count = 0;
    if (argc > 1) {
        for (int i=1; i < argc; i++) {
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            } else {
                double answer = meval(argv[i], &error);
                if (error.type == MEVAL_NO_ERROR) {
                    printf("%f\n", answer);
                } else {
                    printf("%s\n", error.message);
                    failure_count++;
                }
            }
        }
        if (failure_count == 0) {
            exit(EXIT_SUCCESS);
        } else {
            exit(EXIT_FAILURE);
        }
    }
    /*
     * This is a repl now
     */
    printf("Have entered the repl now ...\n");

    //const char* expr = "5 + 4 - (2 / _6)";
    //const char* expr = "5+4-(2/ _6)";
    //const char* expr = "5+4 - (2 / 3)";
    //double value = meval(expr, &error);
    //if (error.type == MEVAL_NO_ERROR) {
    //    printf("expression '%s' evaluated to %f\n", expr, value);
    //}

#ifdef MEVAL_DB_ENABLED
    MEvalVarArr test_vars = {0};
    MEvalVar test_var = {.name="f", .name_char_count=1, .value=3};
    meval_append_variable(&test_vars, test_var);
    DBPRINT("test vars: \n");
    print_vars(test_vars);
    double test_output = meval_var("f+1", test_vars, &error);
    if (error.type == MEVAL_NO_ERROR) {
        printf("test_output: %f\n", test_output);
    } else {
        printf("test_output failed: %s\n", error.message);
    }

    const char* test_compile_expression = "5+12+f";
    MEvalCompiledExpr* compiled_expression = meval_var_compile(test_compile_expression, &error);
    if (error.type == MEVAL_NO_ERROR) {
        printf("expression '%s' has no errors\n", test_compile_expression);
        double compiled_value = meval_var_eval_cexpr(compiled_expression, test_vars, &error);
        if (error.type != MEVAL_NO_ERROR) {
            printf("Compiled expression '%s' evaluation failed, %d:%s\n", test_compile_expression, error.type, error.message);
        } else {
            printf("Compiled expression '%s', evalued to %f\n", test_compile_expression, compiled_value);
        }
    } else {
        printf("expression '%s' had errores: %d:%s\n", test_compile_expression, error.type, error.message);
    }
    meval_free_compiled_expr(&compiled_expression);
#endif
    
    bool include_vars = false;
    double value = 0;
    while (true) {
        /*
        printf("eval >> ");
        char input[REPL_INPUT_BUFFER_LEN] = {0};
        char* input_return = fgets(input, REPL_INPUT_BUFFER_LEN, stdin);
        size_t input_len = strlen(input);
        */
        char input[REPL_INPUT_BUFFER_LEN] = {0};
        size_t input_len;
        get_input_line("eval >> ", input, &input_len);

        char vars_input[REPL_INPUT_BUFFER_LEN] = {0};
        size_t vars_input_len;
        MEvalVarArr vars = {0};
        if (include_vars) {
            get_input_line("vars >> ", vars_input, &vars_input_len);
            vars = parse_for_vars(vars_input, vars_input_len);
            DBPRINT("Found vars: \n");
            print_vars(vars);
        }

        //if (input_len == REPL_INPUT_BUFFER_LEN-1) { // Attempt to clear the stdin buffer if the whole buffer did not fit in the input buffer.
        //    if (input[input_len] != '\n') {
        //        int c = 0;
        //        while (c != '\n' && c != EOF) {
        //            c = getc(stdin);
        //        }
        //    }
        //}
        /*
        if (strcmp("exit\n", input) == 0 || input_return == NULL) {
            printf("\r");
            exit(0);
        }
        */

        if (match_option("vars", input)) {
            include_vars = !include_vars;
            printf("%s\n", include_vars ? "vars enabled" : "vars disabled");
            continue;
        }

        if (include_vars) {
            value = meval_var(input, vars, &error);
            meval_free_variable_arr(&vars);
        } else {
            value = meval(input, &error);
        }

        if (error.type == MEVAL_NO_ERROR) {
            printf("input expression '%s' evaluated to %f\n", input, value);
        } else {
            printf("input expression '%s' had an error, %s\n", input, error.message);
        }
    }
}
