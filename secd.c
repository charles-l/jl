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

long cons(long a, long b) {
    return ((long) cons_(a, b)) | T_CONS;
}

long vint(long v) {
    return v << NSHIFT;
}

long popi() {
    assert(C != NIL);
    long v = car_(C);
    C = (pair) cdr_(C);
    return (long) v;
}

long pop() {
    assert(S != NIL);
    long v = car_(S);
    S = (pair) cdr_(S);
    return (long) v;
}

void push(long v) {
    S = cons_(v, (long) S);
}

void pushd(long v) {
    D = cons_(v, (long) D);
}

long popd() {
    assert(D != NIL);
    long v = car_(D);
    D = (pair) cdr_(D);
    return (long) v;
}

int eval() {
    while(C != (pair) NIL) {
        switch(popi()) {
            case LNIL:
                push((long) NIL);
                break;
            case LDC:
                push(car_(C));
                break;
            case LD:
                {
                    pair v = (pair) popi();
                    assert(car(v) == 1); // TODO: look in surrounding scope
                    assert(E != NIL);
                    int d = cdr(v) - 1;
                    pair p;
                    for(p = (pair) car_(E); d-- && p != NIL; p = (pair) cdr_(p));
                    assert(p != NIL); // TODO: throw error
                    push(car_(p));
                }
                break;
            case SEL:
                {
                    long c = pop();
                    pair t = (pair) popi();
                    pair f = (pair) popi();
                    pushd((long) C); // save the next instruction stream
                    if(c != (long) NIL) {
                        C = t;
                    } else {
                        C = f;
                    }
                }
                break;
            case JOIN:
                {
                    C = (pair) popd();
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
    }
}

// terser pair generation
#define P(car, cdr) cons_(car, (long) cdr)

void reset() {
    S = NIL;
    E = NIL;
    C = NIL;
    D = NIL;
    puts("");
}

void t1() {
    E = P((long) P(8, P(1, NIL)), NIL);
    C = P(LD, P(cons(1, 1), NIL));
    eval();
    print_utlist(S);
}

void t2() {
    C = P(LNIL, P(LDC, P(32,
        P(CONS, NIL))));
    eval();
    print_utlist(S);
}

void t3() {
    C = P(LDC, P(16, P(SEL,
                    P((long) P(LDC, P(16, P(JOIN, NIL))),
                    P((long) P(LNIL, P(JOIN, NIL)),
                        P(LNIL, NIL))))));
    eval();
    print_utlist(S);
}

int main() {
    void (*t[])() = {t1, t2, t3};
    for(int i = 0; i < sizeof(t) / sizeof(void *); i++) {
        // TODO: add assertions for tests
        t[i]();
        reset();
    }
    return 0;
}
