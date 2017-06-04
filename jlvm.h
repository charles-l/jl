#ifndef JLVM_H
#define JLVM_H
#include <assert.h>

#define NSHIFT 3
#define NIL ((pair) 0x1)

#define BITMASK 0b111

typedef enum {
    T_INT  = 0x0,
    T_CONS = 0x1,
    T_FUN  = 0x2,
    T_CFUN = 0x3, // FIXME: remove?
    T_SYM  = 0x4,
} type;

// TODO: add symbol table
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
    PUSHE,// push top of stack onto env list (FIXME: should actually append to end)
    RET,  // pops return value from stack, restores S E C from dump
    DUM,  // push empty list in front of env list (used with RAP)
    CAR,  // cars the top of the stack and pushes it back
    CDR,  // cdrs the top of the stack and pushes it back
    ATOM, // atom? func
    CONS, // conses top two stack elements
    EQ,   // checks equality of top two stack elements
    ADD,  // adds top two stack elements
    SUB,  // subs top two stack elements
    MUL,  // multiplies top two stack elements
    DIV,  // divides top two stack elements
    REM,  // calculates remainder of top two stack elements
    LT,   // determines whether second stack element is less than top element
    CAP,  // c function apply
} op;

typedef long *pair;

#define car_(c) ((c - T_CONS)[0])
#define cdr_(c) ((c - T_CONS)[1])

#define CHECK_TAG(t, v) ((((long) (v)) & BITMASK) == (t))
#define IS_CONS(v) CHECK_TAG(T_CONS, (v))
#define IS_INT(v) CHECK_TAG(T_INT, (v))
#define IS_SYM(v) CHECK_TAG(T_SYM, (v))
#define IS_FUN(v) CHECK_TAG(T_FUN, (v))

long car(pair c) {
    assert(IS_CONS(c));
    return car_(c);
}

long cdr(pair c) {
    assert(IS_CONS(c));
    return cdr_(c);
}

#endif // JLVM_H
