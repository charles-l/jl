#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <execinfo.h>
#include <string.h>
#include "jlvm.h"

// TODO:
//   - decide whether or not there needs to be a special value for true
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

pair cons(long a, long b) {
    pair v = (pair) ((long) malloc(sizeof(long) * 2) | T_CONS);
    car_(v) = a;
    cdr_(v) = b;
    return v;
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

long pop(pair *p) {
    assert(*p != NIL);
    long v = car(*p);
    *p = (pair) cdr(*p);
    return (long) v;
}

void push(pair *p, long v) {
    *p = cons(v, (long) *p);
}

int eval() {
    while(C != (pair) NIL) {
        switch(pop(&C)) {
            case LNIL:
                push(&S, (long) NIL);
                break;
            case LDC: // TODO: split this into multiple instructions (i.e. ld int, ld list, ld str, etc.)
                push(&S, pop(&C));
                break;
            case LD:
                {
                    assert(E != NIL);
                    pair v = (pair) pop(&C);
                    pair s = E;
                    for(int o = car(v);
                            o--;
                            s = (pair) cdr(s));
                    s = (pair) car(s);
                    for(int i = cdr(v);
                            i--;
                            s = (pair) cdr(s));

                    assert(s != NIL && "Failed to find variable");
                    push(&S, car(s));
                }
                break;
            case SEL:
                {
                    long c = pop(&S);
                    pair t = (pair) pop(&C);
                    pair f = (pair) pop(&C);
                    push(&D, (long) C); // save the next instruction stream
                    C = ((c != (long) NIL) ? t : f);
                }
                break;
            case JOIN:
                C = (pair) pop(&D);
                break;
            case LDF:
                push(&S, (long) cons(pop(&C), (long) E) - T_CONS + T_FUN);
                break;
            case AP:
                {
                    pair c = (pair) pop(&S);
                    assert(IS_FUN(c));
                    c = (pair) ((long) c - T_FUN + T_CONS);

                    pair a = (pair) pop(&S);

                    push(&D, (long) S);
                    push(&D, (long) E);
                    push(&D, (long) C);
                    S = NIL;
                    E = cons((long) a, cdr(c));
                    C = (pair) car(c);
                }
                break;
            case TAP:
                {
                    pair c = (pair) pop(&S);
                    assert(IS_FUN(c));
                    c = (pair) ((long) c - T_FUN) + T_CONS;

                    pair a = (pair) pop(&S);

                    S = NIL;
                    E = cons((long) a, cdr(c));
                    C = (pair) car(c);
                }
                break;
            case RAP:
                {
                    pair c = (pair) pop(&S);
                    assert(IS_FUN(c));

                    pair a = (pair) pop(&S);
                    c = (pair) ((long) c - T_FUN + T_CONS);
                    push(&D, (long) S);
                    push(&D, (long) E);
                    push(&D, (long) C);
                    S = NIL;
                    car_(E) = (long) a;
                    C = (pair) car(c);
                }
                break;
            case PUSHE:
                {
                    pair c = (pair) pop(&S);
                    push((pair *) &(car_(E)), (long) c);
                }
                break;
            case RET:
                {
                    long r = pop(&S);
                    C = (pair) pop(&D);
                    E = (pair) pop(&D);
                    S = (pair) pop(&D);
                    push(&S, r);
                }
                break;
            case DUM:
                // TODO: remove NIL + 1 - make it a symbol or something
                E = cons((long) NIL + 1, (long) E);
                break;
            case CONS:
                {
                    long a = pop(&S);
                    long b = pop(&S);
                    push(&S, (long) cons(a, b));
                }
                break;
            case CAR:
                {
                    pair a = (pair) pop(&S);
                    push(&S, (long) car(a));
                }
                break;
            case CDR:
                {
                    pair a = (pair) pop(&S);
                    push(&S, (long) cdr(a));
                }
                break;
            case ATOM:
                {
                    long c = pop(&S);
                    push(&S, IS_INT(c) || IS_SYM(c) ? sym("t") : (long) NIL);
                }
                break;
            case EQ:
                {
                    long a = pop(&S);
                    long b = pop(&S);
                    push(&S, a == b ? sym("t") : (long) NIL);
                }
                break;
            case ADD:
                {
                    long b = pop(&S);
                    long a = pop(&S);
                    assert(IS_INT(a) && IS_INT(b));
                    push(&S, a + b);
                }
                break;
            case SUB:
                {
                    long b = pop(&S);
                    long a = pop(&S);
                    assert(IS_INT(a) && IS_INT(b));
                    push(&S, a - b);
                }
                break;
            case MUL:
                {
                    long b = pop(&S);
                    long a = pop(&S);
                    assert(IS_INT(a) && IS_INT(b));
                    push(&S, ((a >> NSHIFT) * (b >> NSHIFT)) << NSHIFT);
                }
                break;
            case DIV:
                {
                    long b = pop(&S);
                    long a = pop(&S);
                    assert(IS_INT(a) && IS_INT(b));
                    push(&S, ((a >> NSHIFT) / (b >> NSHIFT)) << NSHIFT);
                }
                break;
            case REM:
                {
                    long b = pop(&S);
                    long a = pop(&S);
                    assert(IS_INT(a) && IS_INT(b));
                    push(&S, ((a >> NSHIFT) % (b >> NSHIFT)) << NSHIFT);
                }
                break;
            case LT:
                {
                    long b = pop(&S);
                    long a = pop(&S);
                    assert(IS_INT(a) && IS_INT(b));
                    push(&S, (a < b ? (long) sym("t") : (long) NIL));
                }
                break;
            case CAP:
                // for now this is going to be a direct function pointer, but in the future
                // do some kind of proper reflection to get at the pointer, or pass it
                // around in a lambda of some sort
                {
                    long (*f)(long) = (long (*) (long)) pop(&C);
                    long a = pop(&S);
                    push(&S, f(a));
                }
                break;
        }
    }
}

void eval_bytes(long *b, size_t n) {
    S = NIL;
    E = NIL;
    C = NIL;
    D = NIL;
    for(int i = n - 1; i >= 0; i--) {
        C = cons(b[i], (long) C);
    }
    eval();
}
