#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MEVAL_VERSION_MAJOR 1
#define MEVAL_VERSION_MINOR 0

enum MEVAL_ERROR {MEVAL_NO_ERROR, MEVAL_LEX_ERROR, MEVAL_PARSE_ERROR, MEVAL_PACKAGING_ERROR};
#define MEVAL_ERROR_STRING_LEN 256
#define MEVAL_VAR_NAME_MAX_LEN 32
struct MEvalError {
    enum MEVAL_ERROR type;
    uint32_t char_index;
    char message[MEVAL_ERROR_STRING_LEN];
};

typedef struct {
    char name[MEVAL_VAR_NAME_MAX_LEN];
    uint32_t name_char_count; // CharCount.
    double value;
} MEvalVar;

typedef struct {
    MEvalVar* arr_ptr;
    uint32_t elements_count;
    uint32_t capacity_elements;
} MEvalVarArr;

typedef struct MEvalCompiledExpr MEvalCompiledExpr;

double meval(const char* input_string, struct MEvalError* error);
double meval_var(const char* input_string, MEvalVarArr variables, struct MEvalError* error);
MEvalCompiledExpr* meval_var_compile(const char* input_string, struct MEvalError* output_error);
double meval_var_eval_cexpr(const MEvalCompiledExpr* compiled_expr, MEvalVarArr variables, struct MEvalError* output_error);

bool meval_append_variable(MEvalVarArr *variables_array, MEvalVar new_variable);
void meval_free_variable_arr(MEvalVarArr *variables_array);
void meval_free_compiled_expr(MEvalCompiledExpr** compiled_expr);

struct MEvalTokens; // A array of processed tokens (in their RPN form, allows for quick evaluation)
