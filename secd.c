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
static long TRUE;

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
    LEQ,
    CAP,  // c function apply
} op;

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

// terser pair generation
#define P(car, cdr) cons_(car, (long) cdr)

#define NUMARGS(...)  (sizeof((long[]){__VA_ARGS__})/sizeof(long))
#define LIST_(...) (list_(NUMARGS(__VA_ARGS__), __VA_ARGS__))

// list constructor accepting whatever cons we want to use (tagged, untagged, reversed, etc)
pair list_(int n, ...) {
    va_list l;
    va_start(l, n);
    pair p = cons_(va_arg(l, long), (long) NIL);
    pair c = p;
    for(int i = 0; i < (n - 1); i++) {
        cdr_(c) = (long) cons_(va_arg(l, long), (long) NIL);
        c = (pair) cdr_(c);
    }
    va_end(l);
    return p;
}

void reset() {
    S = NIL;
    E = NIL;
    C = NIL;
    D = NIL;
    puts("");
}

void t1() {
    E = LIST_((long) cons(1 << NSHIFT, (long) NIL));
    C = LIST_(LD, (long) cons_(0, 0));
    eval();
    print_utlist(S);
}

void t2() {
    C = LIST_(LNIL, LDC, 32, CONS);
    eval();
    print_utlist(S);
}

void t3() {
    C = LIST_(LDC, 16,
            SEL,
                (long) LIST_(LDC, 16, JOIN),
                (long) LIST_(LNIL, JOIN), LNIL);
    eval();
    print_utlist(S);
}

void t4() {
    E = LIST_((long) NIL, (long) NIL);
    C = LIST_(LDF,
            (long) LIST_(
                LD, (long) cons_(0, 0),
                LD, (long) cons_(0, 1), RET));
    eval();
    print_utlist(S);
}

void t5() {
    E = LIST_((long) 24, (long) NIL);
    C = LIST_(
            LDC, (long) cons(8, (long) cons(16, (long) NIL)),
            LDF, (long) LIST_(
                LD, (long) cons_(0, 0),
                LD, (long) cons_(0, 1), RET),
            AP);
    eval();
    print_utlist(S);
}

void t6() {
    // ((lambda (x) (x)) (lambda () 1))
    C = LIST_(LNIL,
            LDF, (long) LIST_(LDC, 8, RET), CONS,
            LDF, (long) LIST_(LNIL, LD, (long) cons_(0, 0), AP, RET), AP);
    eval();
    print_utlist(S);
}

void t7() {
    // (def (a) a)
    C = LIST_(DUM,
            LNIL,
            LDF, (long) LIST_(LD, (long) cons_(0, 0), RET), CONS,
            LDF, (long) LIST_(LNIL, LDC, 16, CONS, LD, (long) cons_(0, 0), AP, RET),
            RAP);
    eval();
    print_utlist(S);
}

long tprint(long i) {
    printf("0x%x\n", i);
    return i;
}

void t8() {
    E = LIST_(
            (long) cons(8, (long) cons(16, (long) NIL)),
            (long) cons(32, (long) cons(64, (long) NIL))
            );
    C = LIST_(LD, (long) cons_(0, 0),
              LD, (long) cons_(0, 1),
              LNIL,
              LD, (long) cons_(1, 0), CONS,
              LD, (long) cons_(1, 1), CONS, CDR, CAR,
              LDC, sym("hi"),
              LDC, sym("hi"),
              EQ,
              LDC, 8, LDC, 32, ADD,
              LDC, 32, LDC, 8, SUB,
              LDC, 32, LDC, 16, MUL,
              LDC, 40, LDC, 16, DIV,
              LDC, 40, LDC, 16, REM,
              CAP, (long) &tprint);
    eval();
    print_utlist(S);
}

void t9() {
    // (def (a)
    //  (a))
    C = LIST_(DUM,
            LNIL,
            LDF, (long) LIST_(LNIL, LD, (long) cons_(1, 0), TAP),
            CONS,
            LDF, (long) LIST_(LNIL, LD, (long) cons_(0, 0), AP), RAP);
    eval();
    print_utlist(S);
}

int main() {
    void (*t[])() = {t1, t2, t3, t4, t5, t6, t7, t8};
    for(int i = 0; i < sizeof(t) / sizeof(void *); i++) {
        // TODO: add assertions for tests
        printf("%i: ", i + 1);
        t[i]();
        reset();
    }
    return 0;
}
