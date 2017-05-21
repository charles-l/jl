#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <execinfo.h>
#include <string.h>
#include "vm.h"

// TODO:
//   - get rid of untagged cons, LIST, etc. It's stupid.
//   - decide whether or not there needs to be a special value for true
//   - bignum
//   - floating point
//   - errors

// notes
// - this is an SECD machine
//   - S: holds current stack state
//   - E: holds the environment
//   - C: holds the control (i.e. instruction pointer)
//   - D: holds the dump (basically holds previous state info that will be restored)
// - functions ending in an underscore are return untagged/expect untagged arguments

// registers
static pair S = NIL; // stack pointer
static pair E = NIL; // env pointer
static pair C = NIL; // control/instruction pointer
static pair D = NIL; // dump pointer

pair cons_(long a, long b) {
    pair v = malloc(sizeof(long) * 2);
    v[0] = a;
    v[1] = b;
    return v;
}

pair cons(long a, long b) {
    return (pair) (((long) cons_(a, b)) | T_CONS);
}

long sym(char *s) {
    static pair symbols = NULL;
    for(pair p = symbols; p != NULL; p = (pair) cdr(p)) {
        if(strcmp((char *) car(p), s) == 0) {
            return (car(p) | T_SYM);
        }
    }
    char *r = strdup(s);
    symbols = cons((long) r, (long) symbols);
    return ((long) r | T_SYM);
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

void pushd(pair v) {
    D = cons_((long) v, (long) D);
}

pair popd() {
    assert(D != NIL);
    long v = car_(D);
    D = (pair) cdr_(D);
    return (pair) v;
}

int eval() {
    while(C != (pair) NIL) {
        switch(popi()) {
            case LNIL:
                push((long) NIL);
                break;
            case LDC:
                push(popi(C));
                break;
            case LD:
                {
                    assert(E != NIL);
                    pair v = (pair) popi();
                    pair s = E;
                    for(int o = car_(v);
                            o--;
                            s = (pair) cdr_(s));
                    s = (pair) car_(s);
                    for(int i = cdr_(v);
                            i--;
                            s = (pair) cdr(s));

                    assert(s != NIL && "Failed to find variable");
                    push(car(s));
                }
                break;
            case SEL:
                {
                    long c = pop();
                    pair t = (pair) popi();
                    pair f = (pair) popi();
                    pushd(C); // save the next instruction stream
                    if(c != (long) NIL) {
                        C = t;
                    } else {
                        C = f;
                    }
                }
                break;
            case JOIN:
                C = (pair) popd();
                break;
            case LDF:
                push(((long) cons_(popi(), (long) E)) | T_FUN);
                break;
            case AP:
                {
                    pair c = (pair) pop();
                    assert(IS_FUN(c));
                    c = (pair) ((long) c & ~T_FUN);

                    pair a = (pair) pop();

                    pushd(S);
                    pushd(E);
                    pushd(C);
                    S = NIL;
                    E = cons_((long) a, cdr_(c));
                    C = (pair) car_(c);
                }
                break;
            case TAP:
                {
                    pair c = (pair) pop();
                    assert(IS_FUN(c));
                    c = (pair) ((long) c & ~T_FUN);

                    pair a = (pair) pop();

                    S = NIL;
                    E = cons_((long) a, cdr_(c));
                    C = (pair) car_(c);
                }
                break;
            case RAP:
                {
                    pair c = (pair) pop();
                    assert(IS_FUN(c));

                    pair a = (pair) pop();
                    c = (pair) ((long) c & ~T_FUN);
                    pushd(S);
                    pushd(E);
                    pushd(C);
                    S = NIL;
                    car_(E) = (long) a;
                    C = (pair) car_(c);
                }
                break;
            case RET:
                {
                    long r = pop();
                    C = popd();
                    E = popd();
                    S = popd();
                    push(r);
                }
                break;
            case DUM:
                // TODO: remove NIL + 1 - make it a symbol or something
                E = cons_((long) NIL + 1, (long) E);
                break;
            case CONS:
                {
                    long a = pop();
                    long b = pop();
                    push((long) cons(a, b));
                }
                break;
            case CAR:
                {
                    pair a = (pair) pop();
                    push((long) car(a));
                }
                break;
            case CDR:
                {
                    pair a = (pair) pop();
                    push((long) cdr(a));
                }
                break;
            case ATOM:
                {
                    long c = pop();
                    if(IS_INT(c) || IS_INT(c)) {
                        push(sym("t"));
                    } else {
                        push((long) NIL);
                    }
                }
                break;
            case EQ:
                {
                    long a = pop();
                    long b = pop();
                    if(a == b) {
                        push(sym("t"));
                    } else {
                        push((long) NIL);
                    }
                }
                break;
            case ADD:
                {
                    long b = pop();
                    long a = pop();
                    assert(IS_INT(a) && IS_INT(b));
                    push(a + b);
                }
                break;
            case SUB:
                {
                    long b = pop();
                    long a = pop();
                    assert(IS_INT(a) && IS_INT(b));
                    push(a - b);
                }
                break;
            case MUL:
                {
                    long b = pop();
                    long a = pop();
                    assert(IS_INT(a) && IS_INT(b));
                    push(((a >> NSHIFT) * (b >> NSHIFT)) << NSHIFT);
                }
                break;
            case DIV:
                {
                    long b = pop();
                    long a = pop();
                    assert(IS_INT(a) && IS_INT(b));
                    push(((a >> NSHIFT) / (b >> NSHIFT)) << NSHIFT);
                }
                break;
            case REM:
                {
                    long b = pop();
                    long a = pop();
                    assert(IS_INT(a) && IS_INT(b));
                    push(((a >> NSHIFT) % (b >> NSHIFT)) << NSHIFT);
                }
                break;
            case LT:
                {
                    long b = pop();
                    long a = pop();
                    assert(IS_INT(a) && IS_INT(b));
                    push((a < b ? (long) sym("t") : (long) NIL));
                }
                break;
            case CAP:
                // for now this is going to be a direct function pointer, but in the future
                // do some kind of proper reflection to get at the pointer, or pass it
                // around in a lambda of some sort
                {
                    long (*f)(long) = (long (*) (long)) popi();
                    long a = pop();
                    push(f(a));
                }
                break;
        }
    }
}
