#pragma once
#include <stdint.h>

enum MEVAL_ERROR {MEVAL_NO_ERROR, MEVAL_LEX_ERROR, MEVAL_PARSE_ERROR};
#define MEVAL_ERROR_STRING_LEN 256
struct MEvalError {
    enum MEVAL_ERROR type;
    uint32_t char_index;
    char message[MEVAL_ERROR_STRING_LEN];
};
double meval(const char* input_string, struct MEvalError* error);

typedef struct {
    const char* name;
    uint32_t len; // CharCount.
} MEvalVar;

typedef struct {
    MEvalVar* arr;
    uint32_t len;
} MEvalVarArr;

struct MEvalTokens; // A array of processed tokens (in their RPN form, allows for quick evaluation)
