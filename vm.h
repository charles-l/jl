#ifndef VM_H
#define VM_H
#include <assert.h>

#define NSHIFT 3
#define NIL ((pair) 0x1)

#define BITMASK 0b111

typedef enum {
    T_INT  = 0x0,
    T_CONS = 0x1,
    T_FUN  = 0x2,
    T_CFUN = 0x3,
    T_SYM  = 0x4,
} type;

typedef enum {
    LNIL, // push nil
    LDC,  // push a constant
    LD,   // push value of variable onto stack
    SEL,  // selection statement
    JOIN, // pops from D and makes new value of C
    LDF,  // constructs a closure (env . function)
    AP,   // pops closure and parameter list to apply function
    TAP,  // tail apply (for proper tail-recursion)
    RAP,  // same as AP, but replaces a dummy env with current one for tail calls
    RET,  // pops return value from stack, restores S E C from dump
    DUM,  // push empty list in front of env list (used with RAP)
    CAR,
    CDR,
    ATOM, // atom? func
    CONS,
    EQ,
    ADD,
    SUB,
    MUL,
    DIV,
    REM,
    LT,
    CAP,  // c function apply
} op;

typedef long *pair;

#define car_(c) ((c)[0])
#define cdr_(c) ((c)[1])

#define UNTAGC(c) ((pair) (((long) (c)) & ~T_CONS))
#define CHECK_TAG(t, v) ((((long) (v)) & BITMASK) == (t))
#define IS_CONS(v) CHECK_TAG(T_CONS, (v))
#define IS_INT(v) CHECK_TAG(T_INT, (v))
#define IS_SYM(v) CHECK_TAG(T_SYM, (v))
#define IS_FUN(v) CHECK_TAG(T_FUN, (v))

long car(pair c) {
    assert(IS_CONS(c));
    return car_(UNTAGC(c));
}

long cdr(pair c) {
    assert(IS_CONS(c));
    return cdr_(UNTAGC(c));
}

#endif // VM_H
