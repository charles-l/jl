#include "test.h"
#include "jlvm.h"
#include "jlvm.c"

#define NUMARGS(...)  (sizeof((long[]){__VA_ARGS__})/sizeof(long))
#define LIST_(...) (list_(NUMARGS(__VA_ARGS__), __VA_ARGS__))

void print_val(long v) {
    if(IS_INT(v)) {
        printf("INT<%d>", v >> NSHIFT);
    } else if ((v & BITMASK) == T_SYM) {
        printf("SYM<%s>", v - T_SYM);
    } else if (IS_CONS(v)) {
        pair p = (pair) v;
        if(p != NIL) {
            printf("(");
            print_val(car(p));
            printf(" . ");
            print_val(cdr(p));
            printf(")");
        } else {
            printf("'()");
        }
    } else if (IS_FUN(v)) {
        pair c = (pair) ((long) v & ~T_FUN);
        printf("FUN<frees: ");
        print_utlist((long) cdr_(c));
        printf(" code: %p", car_(c));
        putchar('>');
    } else {
        printf("?<0x%x>", v);
    }
}

void print_utlist(pair l) {
    if(l != NIL) {
        putchar('(');
        _print_utlist(l);
        putchar(')');
    } else {
        printf("'()");
    }
}

void _print_utlist(pair l) {
    if(l != NIL) {
        print_val(car_(l));
        if(cdr_(l) != (long) NIL) {
            putchar(' ');
            _print_utlist((pair) cdr_(l));
        }
    } else {
        printf("'()");
    }
}

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
}

void t1() {
    //liste(LIST_(1, 2, 3), LIST_(1, 2, 3));
    E = LIST_((long) cons(1 << NSHIFT, (long) NIL));
    C = LIST_(LD, (long) cons_(0, 0));
    eval();
    print_utlist(S);
}

void t2() {
    long b[] = {LNIL, LDC, 32, CONS};
    eval_bytes(b, sizeof(b) / sizeof(long));
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
              LDC, 8, LDC, 32, LT,
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

#define CLEAN_TEST(tt) {reset(); TEST(tt)};

//TODO: assert stuff
BEGIN
CLEAN_TEST(t1)
CLEAN_TEST(t2)
CLEAN_TEST(t3)
CLEAN_TEST(t4)
CLEAN_TEST(t5)
CLEAN_TEST(t6)
CLEAN_TEST(t7)
CLEAN_TEST(t8)
//TEST(t9)
END
