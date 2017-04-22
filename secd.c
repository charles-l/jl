#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include "vm.h"

// registers
static pair S = NIL; // stack pointer
static pair E = NIL; // env pointer
static pair C = NIL; // control/instruction pointer
static pair D = NIL; // dump pointer

typedef enum {
    LNIL, // push nil
    LDC,  // push a constant
    LD,   // push value of variable onto stack
    SEL,  // selection statement
    JOIN, // pops from D and makes new value of C
    LDF,  // constructs a closure (env . function)
    AP,   // pops closure and parameter list to apply function
    RET,  // pops return value from stack, restores S E C from dump
    DUM,  // push empty list in front of env list
    RAP,  // same as AP, but replaces a dummy env with current one for tail calls
    CAR,
    CDR,
    CONS,
    CCALL,
} op;

pair cons_(long a, long b) {
    pair v = malloc(sizeof(long) * 2);
    v[0] = a;
    v[1] = b;
    return v;
}

#define TAG_CONS(v) ((long) (v) | T_CONS)
long cons(long a, long b) {
    return TAG_CONS(cons_(a, b));
}

long vint(long v) {
    return v << NSHIFT;
}

long pop() {
    assert(S != NIL);
    long v = car_(S);
    S = (pair) cdr_(S);
    return (long) v;
}

long popi() {
    assert(C != NIL);
    long v = car_(C);
    C = (pair) cdr_(C);
    return (long) C;
}

void push(long v) {
    S = cons_(v, (long) S);
}

int eval() {
    while(C != (pair) NIL) {
        switch(car_(C)) {
            case LNIL:
                push((long) NIL);
                break;
            case LDC:
                C = (pair) cdr_(C);
                push(car_(C));
                break;
            case LD:
                {
                    pair v = (pair) popi();
                    assert(car_(v) == 1); // TODO: look in surrounding scope
                    int d = cdr_(v);
                    pair p;
                    for(p = (pair) car_(E); d-- && p != NIL; p = (pair) cdr_(p));
                    assert(p != NIL); // TODO: throw error
                    push(car_(p));
                }
                break;
            case CONS:
                {
                    long a = pop();
                    long b = pop();
                    push((long) cons(a, b));
                }
                break;
        }
        popi();
    }
}

// terser pair generation
#define P(car, cdr) cons_(car, (long) cdr)

int main() {
    C = P(LNIL,
            P(LDC,
                P(24,
                    P(CONS,
                        P(LNIL,
                            NIL)))));
    /*C = P(LNIL,
            P(LNIL,
                P(LNIL,
                    P(LNIL,
                        P(LNIL,
                            NIL)))));*/
    eval();
    print_utlist(S);
    return 0;
}
