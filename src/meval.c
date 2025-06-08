
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h> // snprintf
#include "meval.h"

enum LEX_TYPE {LT_ERROR, LT_VAR, LT_NUMBER, LT_CONST, LT_UNARY_FUNCTION, LT_BINARY_FUNCTION, LT_OPEN_BRACKET, LT_CLOSE_BRACKET};
enum LEX_ERROR {LE_NONE, LE_UNRECOGNISED_CHAR, LE_UNRECOGNISED_IDENTIFER, LE_MANY_DECIMAL_POINTS};
enum RPN_ERROR {RPNE_NONE, RPNE_FAILED_MEM_ALLOCATION, RPNE_MISSING_OPEN_BRACKET, RPNE_MISSING_CLOSING_BRACKET};
enum EVAL_ERROR {EE_NONE, EE_FAILED_MEM_ALLOCATION, EE_NOT_ENOUGH_OPERANDS /*more functions than operators*/, EE_TOO_MANY_OPERANDS, EE_USE_OF_UNDEFINED_VAR /*function using a undefined variable*/};
#define LEXEAME_CHAR_COUNT 64
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#ifdef MEVAL_DB_ENABLED
#define DBPRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DBPRINT(format, ...)
#endif

typedef struct {
    const char* name;
    uint8_t precedence;
    double (*fnptr)(double);
} UnaryFn;
typedef struct {
    const char* name;
    uint8_t precedence;
    double (*fnptr)(double, double);
} BinaryFn;
typedef struct {
    const char* name;
    double value;
} Constant;

// TODO: Add conditional checks to the following functions (if an error occured, set a global error state somewhere to note of the error). This state should be checked every function call by the caller.

// TODO: Combine all these separate attributes to a single struct.
//  struct UnaryFn(name: str, id: enum/int, precedence: int, fnptr)
//  struct BinaryFn(name: str, id: enum/int, precedence: int, fnptr)
//  struct Constant(name: str, id: enum/int, value: double)

// Note: functions or constants cannot contain both both punctuation and characters in it.
// Note: functions or constants cannot contain digits, although this may change latter.
// Note: functions or constants cannot be (or state with) '(' or ')', these are reserved.
double fn_negate(double a) {return -a;}
double fn_cosec(double a) {return 1/sin(a);}
double fn_sec(double a) {return 1/cos(a);}
double fn_cot(double a) {return 1/tan(a);}
enum UNARY_FUNCTION_NAMES {UFN_NEGATE=0, UFN_SIN, UFN_COS, UFN_TAN, UFN_ASIN, UFN_ACOS, UFN_ATAN, UFN_COSEC, UFN_SEC, UFN_COT, UFN_LOG}; // Used as an index in the 'unary_fns' array.
static UnaryFn unary_fns[] = {
    {.name="_", .precedence=7, .fnptr=fn_negate},
    {.name="sin", .precedence=7, .fnptr=sin},
    {.name="cos", .precedence=7, .fnptr=cos},
    {.name="tan", .precedence=7, .fnptr=tan},
    {.name="asin", .precedence=7, .fnptr=asin},
    {.name="acos", .precedence=7, .fnptr=acos},
    {.name="atan", .precedence=7, .fnptr=atan},
    {.name="cosec", .precedence=7, .fnptr=fn_cosec},
    {.name="sec", .precedence=7, .fnptr=fn_sec},
    {.name="cot", .precedence=7, .fnptr=fn_cot},
    {.name="log", .precedence=7, .fnptr=log}
};
static size_t unary_fn_count = sizeof(unary_fns)/sizeof(UnaryFn); // Seems to be accurate enough. Although if issues occur, just update this manually.

enum BINARY_FUNCTION_NAMES {BFN_ADD=0, BFN_SUB, BFN_MUL, BFN_DIV, BFN_POW, BRN_EQUAL, BRN_GREATER, BRN_LESS, BRN_GREATER_EQUAL, BRN_LESS_EQUAL, BRN_AND, BRN_OR};
double fn_add(double a, double b) {return a+b;}
double fn_sub(double a, double b) {return a-b;}
double fn_mul(double a, double b) {return a*b;}
double fn_div(double a, double b) {return a/b;}
double fn_equal(double a, double b) {return a == b;}
double fn_and(double a, double b) {return a && b;}
double fn_greater(double a, double b) {return a > b;}
double fn_less(double a, double b) {return a < b;}
double fn_greater_equal(double a, double b) {return a >= b;}
double fn_less_equal(double a, double b) {return a <= b;}
double fn_or(double a, double b) {return a || b;}
static BinaryFn binary_fns[] = {
    {.name="+", .precedence=4, .fnptr=fn_add},
    {.name="-", .precedence=4, .fnptr=fn_sub},
    {.name="*", .precedence=5, .fnptr=fn_mul},
    {.name="/", .precedence=5, .fnptr=fn_div},
    {.name="^", .precedence=6, .fnptr=pow},
    {.name="=", .precedence=3, .fnptr=fn_equal},
    {.name=">", .precedence=3, .fnptr=fn_greater},
    {.name="<", .precedence=3, .fnptr=fn_less},
    {.name=">=", .precedence=3, .fnptr=fn_greater_equal},
    {.name="<=", .precedence=3, .fnptr=fn_less_equal},
    {.name="&", .precedence=2, .fnptr=fn_and},
    {.name="|", .precedence=1, .fnptr=fn_or}
};
static size_t binary_fn_count = sizeof(binary_fns)/sizeof(BinaryFn); // Seems to be accurate enough. Although if issues occur, just update this manually.

enum CONSTANT_NAMES {CN_PI=0, CN_E};
static Constant constants[] = {
    {.name="pi", .value=M_PI},
    {.name="e", .value=M_E}
};
static size_t constants_count = sizeof(constants)/sizeof(Constant); // Seems to be accurate enough. Although if issues occur, just update this manually.

typedef struct {
    enum LEX_TYPE type;
    uint32_t char_index;
    /* Potential improvement TODO. Add a lexeame_len therefore would allow for functions to print out the specific text that is has an error, especially as the tokens can now the split, due to partial function name calling support */
    enum LEX_ERROR error_type;
    union {
        char error_str[LEXEAME_CHAR_COUNT]; // Not really needed (The char_index and error_type, contains the same information)
        double number;
        enum UNARY_FUNCTION_NAMES unary_fn;
        enum BINARY_FUNCTION_NAMES binary_fn;
        enum CONSTANT_NAMES const_name;
        char var_name[LEXEAME_CHAR_COUNT];
    } value;
} LexToken;

typedef struct {
    LexToken* tokens_ptr;
    uint32_t tokens_count;
    uint32_t array_capacity;
} LexTokenArray;

const char* get_rpn_error_str(enum RPN_ERROR error) {
    switch (error) {
        case RPNE_NONE:
            return "None";
        case RPNE_FAILED_MEM_ALLOCATION:
            return "Failed Memory Allocation";
        case RPNE_MISSING_OPEN_BRACKET:
            return "Missing Open Bracket";
        case RPNE_MISSING_CLOSING_BRACKET:
            return "Missing Closing Bracket";
        default:
            return "Unknown RPN_ERROR";
    };
}

const char* get_eval_error_str(enum EVAL_ERROR error) {
    switch (error) {
        case EE_NONE:
            return "None";
        case EE_FAILED_MEM_ALLOCATION:
            return "Failed Memory Allocation";
        case EE_NOT_ENOUGH_OPERANDS:
            return "Not Enough Operands";
        case EE_TOO_MANY_OPERANDS:
            return "Too Many Operands";
        case EE_USE_OF_UNDEFINED_VAR:
            return "Use Of Undefined Variable";
        default:
            return "Known EVAL_ERROR";
    };
}

// debug printing
void print_token_value(LexToken token) {
    if (token.type == LT_ERROR) {
        DBPRINT("value=(error_str=%s)", token.value.error_str);
    } else if (token.type == LT_VAR) {
        DBPRINT("value=(var_name=%s)", token.value.var_name);
    } else if (token.type == LT_NUMBER) {
        DBPRINT("value=(number=%f)", token.value.number);
    }  else if (token.type == LT_UNARY_FUNCTION) {
        DBPRINT("value=(unary_function=%d)", token.value.unary_fn);
    } else if (token.type == LT_BINARY_FUNCTION) {
        DBPRINT("value=(binary_function=%d)", token.value.binary_fn);
    } else if (token.type == LT_CONST) {
        DBPRINT("value=(const_name=%d)", token.value.const_name);
    } else {
        DBPRINT("value=(UNKNOWN)");
    }
}
void print_token(LexToken token) {
    DBPRINT("LexToken(type=%d, char_index=%u, error_type=%d, value=...). ", token.type, token.char_index, token.error_type);
    print_token_value(token);
    DBPRINT("\n");
}

bool add_token(LexToken** token_array_ptr, uint32_t* token_array_element_count, uint32_t* token_array_allocated_element_count, LexToken new_token) {
    /* Append the token 'new_token' to the end of the dynamic array '*token_array_ptr' */
    if (*token_array_element_count +1 >= *token_array_allocated_element_count) {
        uint32_t new_allocated_count = *token_array_allocated_element_count*1.5;
        LexToken* tmp = reallocarray(*token_array_ptr, new_allocated_count, sizeof(LexToken));
        if (tmp == NULL) {
            return false;
        }
        *token_array_allocated_element_count = new_allocated_count;
        *token_array_ptr = tmp;
    }
    (*token_array_ptr)[*token_array_element_count] = new_token;
    (*token_array_element_count)++;
    return true;
}

bool match_and_add_char(const char input, const char expected_char, enum LEX_TYPE token_type, uint32_t char_index, LexToken** token_array, uint32_t* tokens_count, uint32_t* tokens_capacity, bool* token_allocation_error) {
    /*
     * Returns true on success, else false.
     *  'token_allocation_error' should be initalized to false, before execution.
     */
    if (input == expected_char) {
        LexToken token = {0};
        token.char_index = char_index;
        token.type = token_type;
        token.error_type = LE_NONE;
        bool success = add_token(token_array, tokens_count, tokens_capacity, token);
        if (!success) {
            *token_allocation_error = true;
        }
        return true;
    }
    return false;
}

void gen_lex_tokens(const char* input_string, uint32_t input_string_char_count, bool allow_variables, LexToken** output_lex_tokens, uint32_t* output_lex_tokens_count, bool* error_occured) {
    /*
     * Input: input_string, input_string_char_count.
     * Output: output_lex_tokens, output_lex_tokens_count, error_occured.
     * Note: error_occured does not get set to false. That is the job of the caller.
     */
    uint32_t output_lex_tokens_allocated_count = 4;
    *output_lex_tokens = malloc(sizeof(LexToken)*output_lex_tokens_allocated_count);
    bool token_handle_error_occured = false;
    for (uint32_t char_index = 0; char_index < input_string_char_count; char_index++) {
        if (isspace(input_string[char_index])) { continue; }
        if (match_and_add_char(input_string[char_index], '(', LT_OPEN_BRACKET, char_index, output_lex_tokens, output_lex_tokens_count, &output_lex_tokens_allocated_count, &token_handle_error_occured)) {
        if (token_handle_error_occured) {
            free(*output_lex_tokens);
            *output_lex_tokens = NULL;
            *error_occured = true;
            return;
        }
        } else if (match_and_add_char(input_string[char_index], ')', LT_CLOSE_BRACKET, char_index, output_lex_tokens, output_lex_tokens_count, &output_lex_tokens_allocated_count, &token_handle_error_occured)) {
            if (token_handle_error_occured) {
                free(*output_lex_tokens);
                *output_lex_tokens = NULL;
                *error_occured = true;
                return;
            }
        } else if (isdigit(input_string[char_index]) || input_string[char_index] == '.') {
            LexToken token = {0};
            token.type = LT_NUMBER;
            token.error_type = LE_NONE;
            token.char_index = char_index;
            const char* start_char = &input_string[char_index];
            uint32_t char_count = 1;
            uint8_t decimal_count = 0;
            if (input_string[char_index] == '.') {
                decimal_count++;
            }
            for (; char_index < input_string_char_count; char_index++) {
                if (isdigit(input_string[char_index])) {
                   char_count++;
                } else if (input_string[char_index] == '.') {
                    decimal_count++;
                    if (decimal_count > 1) {
                        token.type = LT_ERROR;
                        token.error_type = LE_MANY_DECIMAL_POINTS;
                        snprintf(token.value.error_str, LEXEAME_CHAR_COUNT, "[%u] Too many '.' in number", char_index);
                    }
                } else {
                    char_index--;
                    break;
                }
            }
            if (token.type != LT_ERROR) {
                char token_buffer[LEXEAME_CHAR_COUNT] = {0};
                strncpy(token_buffer, start_char, MIN(char_count, LEXEAME_CHAR_COUNT));
                token.value.number = atof(token_buffer);
            }
            bool success = add_token(output_lex_tokens, output_lex_tokens_count, &output_lex_tokens_allocated_count, token);
            if (!success) {
                free(*output_lex_tokens);
                *output_lex_tokens = NULL;
                *error_occured = true;
                return;
            }
            DBPRINT("Added new token: ");
            print_token(token);
        } else if (isalpha(input_string[char_index]) || ispunct(input_string[char_index])) {
            // Add support for variables, (or have a separate lex function that adds support for variabled)
            DBPRINT("Potential identifer...\n");
            bool is_punct = ispunct(input_string[char_index]);
            LexToken token = {0};
            token.type = LT_ERROR;
            token.error_type = LE_UNRECOGNISED_IDENTIFER;
            snprintf(token.value.error_str, LEXEAME_CHAR_COUNT, "[%u] Unknown function or constant", char_index);
            token.char_index = char_index;
            const char* start_char = &input_string[char_index];
            size_t start_char_index = char_index;
            uint32_t char_count = 1;
            bool break_early = false;
            for (char_index++; char_index < input_string_char_count; char_index++) {
                if (input_string[char_index] == '(' || input_string[char_index] == ')') {
                    break_early = true;
                    break;
                }
                if ((isalpha(input_string[char_index]) && !is_punct) || (ispunct(input_string[char_index]) && is_punct)) {
                    char_count++;
                } else {
                    break_early = true;
                    break;
                }
            }
            bool needs_chopping = true;
            uint32_t chopped_char_count = char_count+1;
            uint32_t found_count = 0;
            // TODO: See previous token, if non-existent or a function, then the current function can only be a unary function, therefore ignore binary function checks.
            //   This implements binary-unary function overloading. Also removes evalution error EE_NOT_ENOUGH_OPERANDS (As a binary function can no longer be placed in a unary function location)
            const bool allow_ambiguous_calling = false;
            while (needs_chopping && chopped_char_count > 1) {
                chopped_char_count--;
                for (uint32_t i=0; i < unary_fn_count; i++) {
                    if (strncmp(unary_fns[i].name, start_char, chopped_char_count) == 0) {
                        token.type = LT_UNARY_FUNCTION;
                        token.value.unary_fn = (enum UNARY_FUNCTION_NAMES)i;
                        token.error_type = LE_NONE;
                        needs_chopping = false;
                        //break;
                        if (strlen(unary_fns[i].name) == chopped_char_count || allow_ambiguous_calling) {
                            found_count = 1;
                            break;
                        }
                        found_count++;
                    }
                }
                for (uint32_t i=0; i < binary_fn_count; i++) {
                    if (strncmp(binary_fns[i].name, start_char, chopped_char_count) == 0) {
                        token.type = LT_BINARY_FUNCTION;
                        token.value.binary_fn = (enum BINARY_FUNCTION_NAMES)i;
                        token.error_type = LE_NONE;
                        needs_chopping = false;
                        //break;
                        if (strlen(binary_fns[i].name) == chopped_char_count || allow_ambiguous_calling) {
                            found_count = 1;
                            break;
                        }
                        found_count++;
                    }
                }
                for (uint32_t i=0; i < constants_count; i++) {
                    if (strncmp(constants[i].name, start_char, chopped_char_count) == 0) {
                        token.type = LT_CONST;
                        token.value.const_name = (enum CONSTANT_NAMES)i;
                        token.error_type = LE_NONE;
                        needs_chopping = false;
                        //break;
                        if (strlen(constants[i].name) == chopped_char_count || allow_ambiguous_calling) {
                            found_count = 1;
                            break;
                        }
                        found_count++;
                    }
                }
            }
            if (found_count == 0 && allow_variables) {
                token.type = LT_VAR;
                token.error_type = LE_NONE;
                snprintf(token.value.var_name, MIN(MEVAL_VAR_NAME_MAX_LEN, char_count+1), "%s", start_char);
                DBPRINT("db: Found var with name '%s', at %ld, char_len: %d\n", token.value.var_name, start_char_index, char_count);
            } else if (found_count != 1) { // found_count == 0, iden not found. found_count > 1, iden is ambiguous
                DBPRINT("found_count: %d\n", found_count);
                token.type = LT_ERROR;
                token.error_type = LE_UNRECOGNISED_IDENTIFER;
                snprintf(token.value.error_str, LEXEAME_CHAR_COUNT, "[%u] Unrecognised or ambiguous identifier", char_index);
            }
            if (!needs_chopping) {
                char_index -= char_count - chopped_char_count;
            }
            bool success = add_token(output_lex_tokens, output_lex_tokens_count, &output_lex_tokens_allocated_count, token);
            if (!success) {
                free(*output_lex_tokens);
                *output_lex_tokens = NULL;
                *error_occured = true;
                return;
            }
            if (break_early) { char_index--; }
            if (token.type == LT_ERROR) {
                *error_occured = true;
            }
            DBPRINT("Added new token: ");
            print_token(token);
        } else {
            LexToken token = {0};
            token.char_index = char_index;
            token.type = LT_ERROR;
            token.error_type = LE_UNRECOGNISED_CHAR;
            snprintf(token.value.error_str, LEXEAME_CHAR_COUNT, "[%u] Unknown char", char_index);
            *error_occured = true;
            bool success = add_token(output_lex_tokens, output_lex_tokens_count, &output_lex_tokens_allocated_count, token);
            if (!success) {
                free(*output_lex_tokens);
                *output_lex_tokens = NULL;
                return;
            }
            DBPRINT("Added new token: ");
            print_token(token);
        }
    }
}

uint8_t get_fn_precedence(const LexToken* token_ptr) {
    if (token_ptr->type == LT_UNARY_FUNCTION) {
        return unary_fns[token_ptr->value.unary_fn].precedence;
    } else if (token_ptr->type == LT_BINARY_FUNCTION) {
        return binary_fns[token_ptr->value.binary_fn].precedence;
    }
    return 0;
}

void gen_reverse_polish_notation(const LexToken* input_lex_tokens, const uint32_t lex_token_count, bool allow_variables, LexToken** output_rpn_tokens, uint32_t *output_rpn_tokens_count, enum RPN_ERROR *return_state) { // generate reverse polish notation (might move into lex function)
    //
    // If want support for both binary and unary functions to overlap (such as -), check if the function has two inputs (a LT_NUMBER or LT_CONST (or maybe a bracket) on either side, if there is only one, the treat as a unary function, else as a binary function).
    *output_rpn_tokens_count = 0;
    uint32_t rpn_tokens_capcity = lex_token_count;
    *output_rpn_tokens = malloc(rpn_tokens_capcity*sizeof(LexToken));
    uint32_t token_stack_capacity = 4;
    uint32_t token_stack_count = 0;
    LexToken* token_stack = malloc(token_stack_capacity*sizeof(LexToken));
    if (*output_rpn_tokens == NULL || token_stack == NULL) {
        free(*output_rpn_tokens); // If NULL does nothing.
        free(token_stack);
        *return_state = RPNE_FAILED_MEM_ALLOCATION;
        return;
    }
    int32_t open_bracket_count = 0;
    for (uint32_t input_tokens_index = 0; input_tokens_index < lex_token_count; input_tokens_index++) {
        const LexToken* current_token = &input_lex_tokens[input_tokens_index];
        if (current_token->type == LT_NUMBER || current_token->type == LT_CONST || (current_token->type == LT_VAR && allow_variables)) {
            DBPRINT("Pushing number/const(/var if %d==true) into rpn output\n", allow_variables);
            bool success = add_token(output_rpn_tokens, output_rpn_tokens_count, &rpn_tokens_capcity, *current_token);
            if (!success) {
                free(token_stack);
                *return_state = RPNE_FAILED_MEM_ALLOCATION;
                return;
            }
        } else if (current_token->type == LT_OPEN_BRACKET) {
            DBPRINT("Pushing ( i=%d into token stack\n", current_token->char_index);
            DBPRINT("  open_bracket_count: %d\n", open_bracket_count);
            open_bracket_count++;
            bool success = add_token(&token_stack, &token_stack_count, &token_stack_capacity, *current_token);
            if (!success) {
                free(token_stack);
                *return_state = RPNE_FAILED_MEM_ALLOCATION;
                return;
            }
        } else if (current_token->type == LT_CLOSE_BRACKET) {
            DBPRINT("Found ) in input, now handling it ...\n");
            DBPRINT("  Current open_bracket_count: %d\n", open_bracket_count);
            open_bracket_count--;
            /*
            if (token_stack_count == 0) {
                *return_state = RPNE_MISSING_OPEN_BRACKET;
                free(token_stack);
                return;
            }
            */
            while (true) {
                if (token_stack_count == 0) {
                    // missing an opening bracket (reached end of array, without a open bracket)
                    DBPRINT("Mising open bracket, count: %d\n", open_bracket_count);
                    *return_state = RPNE_MISSING_OPEN_BRACKET;
                    free(token_stack);
                    return;
                }
                current_token = &token_stack[token_stack_count-1];
                if (current_token->type == LT_OPEN_BRACKET) { // Only used as a marker on where to stop
                    DBPRINT("  Found ( i=%d in closing bracket search, ending proccessing\n", current_token->char_index);
                    break;
                }
                DBPRINT("  token (i=%d, t=%d) being added to output token stack\n", current_token->char_index, current_token->type);
                bool success = add_token(output_rpn_tokens, output_rpn_tokens_count, &rpn_tokens_capcity, *current_token);
                if (!success) {
                    free(token_stack);
                    *return_state = RPNE_FAILED_MEM_ALLOCATION;
                    return;
                }
                token_stack_count--;
            }
        } else if (current_token->type == LT_UNARY_FUNCTION || current_token->type == LT_BINARY_FUNCTION) {
            DBPRINT("Pushing function (type=%d) to token stack, ", current_token->type);
            print_token_value(*current_token);
            DBPRINT("\n");
            uint32_t stack_top_precedence = 0;
            if (token_stack_count > 0) {
                stack_top_precedence = get_fn_precedence(&token_stack[token_stack_count-1]);
            }
            uint32_t current_precedence = get_fn_precedence(current_token);
            bool success = true;
            while (stack_top_precedence >= current_precedence) { // Convert to a (eval from right first) like an array language, by changing the condition from '>=' to '>'.
                if (token_stack_count == 0) {
                    break;
                }
                if (token_stack[token_stack_count-1].type == LT_OPEN_BRACKET) {
                    DBPRINT("  found ( on token stack (index=%d), ending function search processing\n", token_stack_count-1);
                    token_stack_count--;
                    break;
                }
                DBPRINT("  moving token (i=%d, t=%d) from token stack to rpn output\n", token_stack[token_stack_count-1].char_index, token_stack[token_stack_count-1].type);
                stack_top_precedence = get_fn_precedence(&token_stack[token_stack_count-1]);
                success = add_token(output_rpn_tokens, output_rpn_tokens_count, &rpn_tokens_capcity, token_stack[token_stack_count-1]);
                if (!success) {
                    free(token_stack);
                    *return_state = RPNE_FAILED_MEM_ALLOCATION;
                    return;
                }
                token_stack_count--;
            }
            success = add_token(&token_stack, &token_stack_count, &token_stack_capacity, *current_token);
            if (!success) {
                free(token_stack);
                *return_state = RPNE_FAILED_MEM_ALLOCATION;
                return;
            }
        }
    }
    for (uint32_t i=token_stack_count-1; i+1 != 0; i--) {
        // pop all the remaining tokens from the tokens stack.
        DBPRINT("Poping remaining token (i=%d, t=%d)\n", token_stack[i].char_index, token_stack[i].type);
        if (token_stack[i].type == LT_OPEN_BRACKET) {
            DBPRINT(" Ignoring open bracket at stack index %d\n", i);
            continue; // Assume the closing bracket was ment to be at the end.
        }
        add_token(output_rpn_tokens, output_rpn_tokens_count, &rpn_tokens_capcity, token_stack[i]);
    }
    free(token_stack);
}

void eval_rpn_tokens(const LexToken* input_rpn_tokens, const uint32_t input_rpn_token_count, bool allow_variables, const MEvalVar* variables_array_ptr, const uint32_t variables_array_element_count, double* output_value, enum EVAL_ERROR *return_state) {
    *output_value = 0;
    *return_state = EE_NONE;
    uint32_t number_stack_count = 0;
    uint32_t number_stack_capacity = 8;
    LexToken* number_stack = malloc(number_stack_capacity*sizeof(LexToken));
    if (number_stack == NULL) {
        *return_state = EE_FAILED_MEM_ALLOCATION;
        return;
    }
    bool success = true;
    for (uint32_t input_tokens_index = 0; input_tokens_index < input_rpn_token_count; input_tokens_index++) {
        const LexToken* current_token = &input_rpn_tokens[input_tokens_index];
        if (current_token->type == LT_NUMBER) {
            success = add_token(&number_stack, &number_stack_count, &number_stack_capacity, *current_token);
            if (!success) {
                *return_state = EE_FAILED_MEM_ALLOCATION;
                free(number_stack);
                return;
            }
        } else if (current_token->type == LT_CONST) {
            LexToken new_token = *current_token;
            new_token.type = LT_NUMBER;
            new_token.value.number = 0;
            for (size_t i=0; i < constants_count; i++) {
                if ((size_t)current_token->value.const_name == i) {
                    new_token.value.number = constants[i].value;
                    break;
                }
            }
            success = add_token(&number_stack, &number_stack_count, &number_stack_capacity, new_token);
            if (!success) {
                *return_state = EE_FAILED_MEM_ALLOCATION;
                free(number_stack);
                return;
            }
        } else if (current_token->type == LT_VAR && allow_variables) {
            int32_t var_index = -1;
            DBPRINT("db: checking against %d variables\n", variables_array_element_count);
            for (size_t i=0; i < variables_array_element_count; i++) {
                DBPRINT("db:  Checking variable '%s'\n", variables_array_ptr[i].name);
                if (strcmp(current_token->value.var_name, variables_array_ptr[i].name) == 0) {
                    var_index = i;
                    break;
                }
            }
            if (var_index == -1) {
                DBPRINT("db: Could not find variable with name '%s', but used in expression\n", current_token->value.var_name);
                *return_state = EE_USE_OF_UNDEFINED_VAR;
                free(number_stack);
                return;
            }
            LexToken new_token = *current_token;
            new_token.type = LT_NUMBER;
            new_token.value.number = variables_array_ptr[var_index].value;
            success = add_token(&number_stack, &number_stack_count, &number_stack_capacity, new_token);
            if (!success) {
                *return_state = EE_FAILED_MEM_ALLOCATION;
                free(number_stack);
                return;
            }
        } else if (current_token->type == LT_UNARY_FUNCTION) {
            if (number_stack_count < 1) {
                *return_state = EE_NOT_ENOUGH_OPERANDS;
                free(number_stack);
                return;
            }
            LexToken evaluated_value = {0};
            evaluated_value.type = LT_NUMBER;
            evaluated_value.char_index = current_token->char_index; // Give the char_index, the functions char_index that it was evaluated from.
            evaluated_value.error_type = LE_NONE;
            double value = number_stack[--number_stack_count].value.number; // Assume this token is a number token.
            evaluated_value.value.number = unary_fns[current_token->value.unary_fn].fnptr(value);
            success = add_token(&number_stack, &number_stack_count, &number_stack_capacity, evaluated_value);
            if (!success) {
                *return_state = EE_FAILED_MEM_ALLOCATION;
                free(number_stack);
                return;
            }
            // use the precedence to determine what to do with this
        } else if (current_token->type == LT_BINARY_FUNCTION) {
            if (number_stack_count < 2) {
                *return_state = EE_NOT_ENOUGH_OPERANDS;
                free(number_stack);
                return;
            }
            LexToken evaluated_value = {0};
            evaluated_value.type = LT_NUMBER;
            evaluated_value.char_index = current_token->char_index; // Give the char_index, the functions char_index that it was evaluated from.
            evaluated_value.error_type = LE_NONE;
            double value_b = number_stack[--number_stack_count].value.number; // Assume token is a valid number, if it is in the number stack.
            double value_a = number_stack[--number_stack_count].value.number; // Assume token is a valid number, if it is in the number stack.
            evaluated_value.value.number = binary_fns[current_token->value.binary_fn].fnptr(value_a, value_b);
            success = add_token(&number_stack, &number_stack_count, &number_stack_capacity, evaluated_value);
            if (!success) {
                *return_state = EE_FAILED_MEM_ALLOCATION;
                free(number_stack);
                return;
            }
            // use the precedence to determine what to do with this
        } // Ignore unknown types (these should have been handled by an earlier stage). I guess this now includes variables.
    }
    if (number_stack_count != 1) {
        *return_state = EE_TOO_MANY_OPERANDS;
        free(number_stack);
        return;
    }
    // TODO: Go through every element, and make sure that a binary function does not have another binary function adjacent (directly next to it).
    //    Unary functions can be ignored, because unary functions can have parameters from the left or the right, or may even have another unary function next to it.
    *output_value = number_stack[0].value.number;
    free(number_stack);
}

bool meval_append_variable(MEvalVarArr *variables_array, MEvalVar new_variable) {
    if (variables_array->elements_count >= variables_array->capacity_elements) {
        uint32_t new_capacity = MAX(variables_array->capacity_elements * 1.5, 3);
        MEvalVar* tmp = reallocarray(variables_array->arr_ptr, new_capacity, sizeof(MEvalVar));
        if (tmp == NULL) {
            return false;
        }
        variables_array->arr_ptr = tmp;
        variables_array->capacity_elements = new_capacity;
    }
    variables_array->arr_ptr[variables_array->elements_count] = new_variable;
    variables_array->elements_count++;
    return true;
}

void free_variable_arr(MEvalVarArr *variables_array) {
    free(variables_array->arr_ptr);
    variables_array->elements_count = 0;
    variables_array->capacity_elements = 0;
}

double meval_internal(const char* input_string, bool support_variables, MEvalVarArr variables, struct MEvalError* output_error) {
    if (input_string == NULL) {
        output_error->type = MEVAL_LEX_ERROR;
        output_error->char_index = 0;
        strncpy(output_error->message, "No Input Given", MEVAL_ERROR_STRING_LEN);
        output_error->message[MEVAL_ERROR_STRING_LEN-1] = '\0';
        return 0;
    }
    LexToken* tokens = NULL;
    uint32_t lex_tokens_count = 0;
    bool error_occured = false;
    const char* error_string = NULL;
    gen_lex_tokens(input_string, strlen(input_string), support_variables, &tokens, &lex_tokens_count, &error_occured);
    if (lex_tokens_count == 0) {
        output_error->type = MEVAL_LEX_ERROR;
        output_error->char_index = 0;
        strncpy(output_error->message, "Empty/Invalid Text Input", MEVAL_ERROR_STRING_LEN);
        output_error->message[MEVAL_ERROR_STRING_LEN-1] = '\0';
        return 0;
    }
    DBPRINT("%d tokens emitted, error_occured: %d\n", lex_tokens_count, error_occured);
    for (size_t i=0; i < lex_tokens_count; i++) {
        print_token(tokens[i]);
    }
    if (error_occured) {
        for (size_t i=0; i < lex_tokens_count; i++) {
            if (tokens[i].type == LT_ERROR) {
                output_error->type = MEVAL_LEX_ERROR;
                output_error->char_index = tokens[i].char_index;
                strncpy(output_error->message, tokens[i].value.error_str, MEVAL_ERROR_STRING_LEN);
                output_error->message[MEVAL_ERROR_STRING_LEN-1] = '\0';
                return 0;
            }
        }
        // Error occured ... But cannot find it?????
    }
    enum RPN_ERROR rpn_error = RPNE_NONE;
    LexToken* rpn_tokens = NULL;
    uint32_t rpn_tokens_count = 0;
    gen_reverse_polish_notation(tokens, lex_tokens_count, support_variables, &rpn_tokens, &rpn_tokens_count, &rpn_error);
    for (size_t i=0; i < rpn_tokens_count; i++) {
        DBPRINT("RPN Token: ");
        print_token(rpn_tokens[i]);
    }
    if (rpn_error != RPNE_NONE) {
        DBPRINT("RPN Error occured (%d)\n", rpn_error);
        output_error->type = MEVAL_PARSE_ERROR;
        if (rpn_tokens_count != 0) {
            output_error->char_index = rpn_tokens[rpn_tokens_count-1].char_index;
        } else {
            output_error->char_index = 0;
        }
        error_string = get_rpn_error_str(rpn_error);
        strncpy(output_error->message, error_string, MEVAL_ERROR_STRING_LEN);
        output_error->message[MEVAL_ERROR_STRING_LEN-1] = '\0';
        return 0;
    }

    double output = 0;
    enum EVAL_ERROR eval_error = EE_NONE;
    eval_rpn_tokens(rpn_tokens, rpn_tokens_count, support_variables, variables.arr_ptr, variables.elements_count, &output, &eval_error);
    if (eval_error != EE_NONE) {
        output_error->type = MEVAL_PARSE_ERROR;
        output_error->char_index = 0; // To be determined.
        error_string = get_eval_error_str(eval_error);
        strncpy(output_error->message, error_string, MEVAL_ERROR_STRING_LEN);
        output_error->message[MEVAL_ERROR_STRING_LEN-1] = '\0';
        return 0;
    }
    DBPRINT("Eval Error: %d\n", eval_error);
    free(rpn_tokens);
    free(tokens);
    // Reset the error object to a known state.
    output_error->type = MEVAL_NO_ERROR;
    output_error->char_index = 0;
    memset(output_error->message, 0, MEVAL_ERROR_STRING_LEN);
    
    return output;
}

double meval(const char* input_string, struct MEvalError* error) {
    MEvalVarArr empty_variables = {0};
    return meval_internal(input_string, false, empty_variables, error);
}

double meval_var(const char* input_string, MEvalVarArr variables, struct MEvalError* error) {
    return meval_internal(input_string, true, variables, error);
}
