% libmeval(3) | Library Functions Manual
# NAME

libmeval - C library for evaluating a math expression to a number

# LIBRARY

Math evaluator library (libmeval, -lmeval)

# SYNOPSIS

```C
#include <meval/meval.h>

double meval(const char* input_string, MEvalError* error);
double meval_var(const char* input_string, const MEvalVarArr variables, MEvalError* error);
MEvalCompiledExpr* meval_var_compile(const char* input_string, MEvalError* output_error);
double meval_var_eval_cexpr(const MEvalCompiledExpr* compiled_expr, const MEvalVarArr variables, MEvalError* output_error);
bool meval_append_variable(MEvalVarArr *variables_array, MEvalVar new_variable);
void meval_free_variable_arr(MEvalVarArr *variables_array);
void meval_free_compiled_expr(MEvalCompiledExpr** compiled_expr);
```

# VERSION

2.5

# ENUMS

## `MEVAL_ERROR`

- `MEVAL_NO_ERROR`          - No error occurred.
- `MEVAL_LEX_ERROR`         - Lexical error occurred.
- `MEVAL_PARSE_ERROR`       - Parser error occurred.
- `MEVAL_PACKAGING_ERROR`   - Failure when generating the `MEvalCompiledExpr` opaque struct.

# PREDEFINED PREPROCESSORS

- `MEVAL_VERSION_MAJOR`      -  Libraries major version number  (INT)
- `MEVAL_VERSION_MINOR`      -  Libraries minor version number  (INT)
- `MEVAL_ERROR_STRING_LEN`   -  Largest error string length (including null byte)  (INT)
- `MEVAL_VAR_NAME_MAX_LEN`   -  Largest variable string length (including null byte)  (INT)

# OPTIONAL DEFINABLE PREPROCESSORS

- `#define MEVAL_MALLOC(x) malloc(x)`
    - Overrides the libraries use of `malloc( ... )`.
- `#define MEVAL_REALLOCARRAY(ptr, nmemb, size) reallocarray(ptr, nmemb, size)`
    - Overrides the libraries use of `reallocarray( ... )`.
- `#define MEVAL_FREE(ptr) free(ptr)`
    - Overrides the libraries use of `free( ... )`.

Each macro definable on their own.
Must be defined through the compiler, due to the way `meval.c` includes `meval.h`.

# `MEvalError` struct

```C
typedef struct MEvalError {
    enum MEVAL_ERROR type;
    uint32_t char_index; /* Index into the original given expression input string */
    char message[MEVAL_ERROR_STRING_LEN]; /* Error message as a string, usually user friendly */
} MEvalError;
```

# `MEvalVar` struct

```C
typedef struct {
    char name[MEVAL_VAR_NAME_MAX_LEN]; /* The variables identifier */
    uint32_t name_char_count; /* The length of the variables identifier. Ignored by meval internally */
    double value; /* The number that the variable holds */
} MEvalVar;
```

# `MEvalVarArr` struct

```C
typedef struct {
    MEvalVar* arr_ptr;
    uint32_t elements_count;
    uint32_t capacity_elements;
} MEvalVarArr;
```

# `MEvalCompiledExpr` opaque struct

```C
struct MEvalCompiledExpr { ... };
typedef struct MEvalCompiledExpr MEvalCompiledExpr;
```

# FUNCTIONS DESCRIPTION

- `double meval(const char* input_string, MEvalError* error);`
    - Simple interface function. Evaluates `input_string` as a expression.
    - `error` is an output variable that always gets set by the function, even on success.
    - Returns the evaluated value, or 0.0f on error.
- `double meval_var(const char* input_string, const MEvalVarArr variables, MEvalError* error);`
    - Same as `meval( ... )` except it supports variables being defined, and therefore used within the expression.
    - Parameter `variables` maybe an empty array, in which case the function behaves exactly like `meval( ... )`.
    - Returns the evaluated value, or 0.0f on error.
    - `error` is an output variable that always gets set by the function, even on success.
    - *NOTE* Internal function names takes precedence over variable names. Any colliding variable name would be ignored.
- `MEvalCompiledExpr* meval_var_compile(const char* input_string, MEvalError* output_error);`
    - Compiles the expression into a more efficient format for repeated evaluation.
    - Any unmatched identifiers in `input_string` are assumed to be variables.
    - Returns a pointer to an opaque struct that represents the compiled version of the expression.
    - The returned `MEvalCompiledExpr*` must be freed, even if the function fails.
    - `output_error` is an output variable that always gets set by the function, even on success.
    - *NOTE* Internal function names takes precedence over variable names. Any colliding variable name would be ignored.
- `double meval_var_eval_cexpr(const MEvalCompiledExpr* compiled_expr, const MEvalVarArr variables, MEvalError* output_error);`
    - Evaluates the given compiled expression, `compiled_expr`, got from `meval_var_compile`.
    - Parameter `variables` maybe an empty array, in which case the function treats all unknown identifiers in the original expression as errors.
    - Returns the evaluated value, or 0.0f on error.
    - `output_error` is an output variable that always gets set by the function, even on success.
    - *NOTE* Internal function names takes precedence over variable names. Any colliding variable name would be ignored.
- `bool meval_append_variable(MEvalVarArr *variables_array, MEvalVar new_variable);`
    - Appends `new_variable` to the end of `variables_array`.
    - Parameter `variables_array` maybe an empty array.
    - Returns `true` when `new_variable` has been successfully appended to the end of `variables_array`. Otherwise returns false on failure.
- `void meval_free_variable_arr(MEvalVarArr *variables_array);`
    - Free any heap allocated memory associated with a given `variables_array`.
    - Calling this function with an empty array is safe.
    - Parameter `variables_array` cannot be `NULL`.
    - Parameter `variables_array` always gets modified by the function, even if `variables_array` is an empty array.
- `void meval_free_compiled_expr(MEvalCompiledExpr** compiled_expr);`
    - Free any heap allocated memory associated with a given `compiled_expr`.
    - Calling this function with an already freed `compiled_expr` is safe.
    - Parameter `compiled_expr` cannot be `NULL`.
    - Parameter `compiled_expr` gets modified by the function if it's not already been freed.

# EXAMPLES

```C
#include <stdio.h>
#include <meval/meval.h>

int main(void) {
    /* Simple library interface */
    MEvalError e;
    double val = meval("1+2", &e);
    if (e.type != MEVAL_NO_ERROR) {
        printf("Failed to evaluate: %s\n", e.message);
        return 1;
    }
    printf("val: %f\n", val);
    
    /* Compiled expression and Variable interface */
    MEvalCompiledExpr* compile_expr = meval_var_compile("2-3*x", &e);
    if (e.type != MEVAL_NO_ERROR) {
        printf("Failed to evaluate: %s\n", e.message);
        return 2;
    }
    MEvalVarArr vars = {0};
    MEvalVar x = {0};
    x.name[0] = 'x';
    x.name[1] = '\0';
    x.value = 12;
    if (meval_append_variable(&vars, x) == false) { return 3; }
    val = meval_var_eval_cexpr(compile_expr, vars, &e);
    if (e.type != MEVAL_NO_ERROR) {
        printf("Failed to evalute: %s\n", e.message);
        return 4;
    }
    meval_free_variable_arr(&vars);
    meval_free_compiled_expr(&compile_expr);
    printf("val: %f\n", val);
}
```

# NOTES

- This library required the standard math library `libm`.
- This library requires the standard C library `libc`.
