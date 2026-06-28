#pragma once

// implementation configuration (iconfig.h)

/* This file may change dramatically at any time, even when part of a stable
 * release, as it is a direct dependency on the internal implementation of the
 * library */

/* This is an internal implementation header file *ONLY*. No external program
 * should include this file, only `meval.c` part of the meval library internal
 * implementation */

// Note: A function or constant cannot contain a mixture of punctuation and
//       letter within it.
// Note: A function or constant cannot contain digits, although this may change
//       latter.
// Note: A function or constant cannot be (or start with) '(' or ')', these
//       characters are reserved.

// Allow for implicity '(' in expressions, set to 1 to allow
#define MEVAL_OPT_ALLOW_MISSING_OPEN_BRACKET 0

static double fn_negate(double a) {return -a;}
static double fn_cosec(double a) {return 1/sin(a);}
static double fn_sec(double a) {return 1/cos(a);}
static double fn_cot(double a) {return 1/tan(a);}

// Enum 'UNARY_FUNCTION_NAMES', used as an index in the 'unary_fns' array.
enum UNARY_FUNCTION_NAMES {UFN_NEGATE=0, UFN_SIN, UFN_COS, UFN_TAN, UFN_ASIN,
    UFN_ACOS, UFN_ATAN, UFN_COSEC, UFN_SEC, UFN_COT, UFN_LOG}; 
static UnaryFn unary_fns[] = {
//  function name    precedence     function pointer
    {.name="_",     .precedence=7, .fnptr=fn_negate},
    {.name="sin",   .precedence=7, .fnptr=sin},
    {.name="cos",   .precedence=7, .fnptr=cos},
    {.name="tan",   .precedence=7, .fnptr=tan},
    {.name="asin",  .precedence=7, .fnptr=asin},
    {.name="acos",  .precedence=7, .fnptr=acos},
    {.name="atan",  .precedence=7, .fnptr=atan},
    {.name="cosec", .precedence=7, .fnptr=fn_cosec},
    {.name="sec",   .precedence=7, .fnptr=fn_sec},
    {.name="cot",   .precedence=7, .fnptr=fn_cot},
    {.name="log",   .precedence=7, .fnptr=log}
};

static double fn_add(double a, double b) {return a+b;}
static double fn_sub(double a, double b) {return a-b;}
static double fn_mul(double a, double b) {return a*b;}
static double fn_div(double a, double b) {return a/b;}
static double fn_mod(double a, double b) {return (size_t)a%(size_t)b;}
static double fn_equal(double a, double b) {return a == b;}
static double fn_greater(double a, double b) {return a > b;}
static double fn_less(double a, double b) {return a < b;}
static double fn_greater_equal(double a, double b) {return a >= b;}
static double fn_less_equal(double a, double b) {return a <= b;}
static double fn_and(double a, double b) {return a && b;}
static double fn_or(double a, double b) {return a || b;}

// Enum 'BINARY_FUNCTION_NAMES', used as an index in the 'binary_fns' array.
enum BINARY_FUNCTION_NAMES {BFN_ADD=0, BFN_SUB, BFN_MUL, BFN_DIV, BFN_MOD,
    BFN_POW, BFN_EQUAL, BFN_GREATER, BFN_LESS, BFN_GREATER_EQUAL,
    BFN_LESS_EQUAL, BFN_AND, BFN_OR};
static BinaryFn binary_fns[] = {
//  function name   precedence     function pointer
    {.name="+",    .precedence=4, .fnptr=fn_add},
    {.name="-",    .precedence=4, .fnptr=fn_sub},
    {.name="*",    .precedence=5, .fnptr=fn_mul},
    {.name="/",    .precedence=5, .fnptr=fn_div},
    {.name="%",    .precedence=5, .fnptr=fn_mod},
    {.name="^",    .precedence=6, .fnptr=pow},
    {.name="=",    .precedence=3, .fnptr=fn_equal},
    {.name=">",    .precedence=3, .fnptr=fn_greater},
    {.name="<",    .precedence=3, .fnptr=fn_less},
    {.name=">=",   .precedence=3, .fnptr=fn_greater_equal},
    {.name="<=",   .precedence=3, .fnptr=fn_less_equal},
    {.name="&",    .precedence=2, .fnptr=fn_and},
    {.name="|",    .precedence=1, .fnptr=fn_or}
};

enum CONSTANT_NAMES {CN_PI=0, CN_E};
static Constant constants[] = {
//  constant name   constant value
    {.name="pi",   .value=M_PI},
    {.name="e",    .value=M_E}
};
