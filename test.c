#include "test.h"
#include "jlvm.h"
#include "jlvm.c"

#define NUMARGS(...)  (sizeof((long[]){__VA_ARGS__})/sizeof(long))
#define LIST(...) (list_(NUMARGS(__VA_ARGS__), __VA_ARGS__))
#define I(i) (i << NSHIFT)

int listcmp(pair x, pair y) {
    for(pair a = (pair) x, b = (pair) y; a != NIL || b != NIL; a = (pair) cdr(a), b = (pair) cdr(b)) {
        if(IS_CONS(car(a)) && IS_CONS(car(b))) {
            return listcmp((pair) car(a), (pair) car(b));
        }
        if(car(a) != car(b)) {
            return 1;
        }
    }
    return 0;
}
#define listeq(x, y) if (listcmp(x, y) != 0) {\
    printf("expected: "); \
    print_val((long) x); \
    printf("\ngot: "); \
    print_val((long) y); \
    printf("\n"); \
    fail; \
}

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
        pair c = (pair) ((long) v - T_FUN + T_CONS);
        printf("FUN<frees: ");
        print_val((long) cdr(c));
        printf(" code: %p", car(c));
        putchar('>');
    } else { printf("?<0x%x>", v);
    }
}
// list constructor accepting whatever cons we want to use (tagged, untagged, reversed, etc)
pair list_(int n, ...) {
    va_list l;
    va_start(l, n);
    pair p = cons(va_arg(l, long), (long) NIL);
    pair c = p;
    for(int i = 0; i < (n - 1); i++) {
        cdr_(c) = (long) cons(va_arg(l, long), (long) NIL);
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
    E = LIST((long) cons(I(1), (long) NIL));
    C = LIST(LD, (long) cons(0, 0));
    eval();
    listeq(LIST(I(1)), S);
}

void t2() {
    long b[] = {LNIL, LDC, I(4), CONS};
    eval_bytes(b, sizeof(b) / sizeof(long));
    listeq(LIST((long) LIST(I(4))), S);
}

void t3() {
    C = LIST(LDC, I(2),
            SEL,
            (long) LIST(LDC, I(2), JOIN),
            (long) LIST(LNIL, JOIN), LNIL);
    eval();
    listeq(LIST((long) NIL, I(2)), S);
}

void t4() {
    E = LIST((long) NIL, (long) NIL);
    C = LIST(LDF,
            (long) LIST(
                LD, (long) cons(0, 0),
                LD, (long) cons(0, 1), RET));
    eval();
    assert(IS_FUN(car(S)));
}

void t5() {
    E = LIST((long) I(3), (long) NIL);
    C = LIST(
            LDC, (long) cons(I(1), (long) cons(I(2), (long) NIL)),
            LDF, (long) LIST(
                LD, (long) cons(0, 0),
                LD, (long) cons(0, 1), RET),
            AP);
    eval();
    listeq(LIST(I(2)), S);
}

void t6() {
    // ((lambda (x) (x)) (lambda () 1))
    C = LIST(LNIL,
            LDF, (long) LIST(LDC, I(1), RET), CONS,
            LDF, (long) LIST(LNIL, LD, (long) cons(0, 0), AP, RET), AP);
    eval();
    listeq(LIST(I(1)), S);
}

void t7() {
    // (def (a) a)
    C = LIST(DUM,
            LNIL,
            LDF, (long) LIST(LD, (long) cons(0, 0), RET), CONS,
            LDF, (long) LIST(LNIL, LDC, I(2), CONS, LD, (long) cons(0, 0), AP, RET),
            RAP);
    eval();
    listeq(LIST(I(2)), S);
}

void t8() {
    E = LIST(
            (long) cons(I(1), (long) cons(I(2), (long) NIL)),
            (long) cons(I(4), (long) cons(I(8), (long) NIL))
            );
    C = LIST(LD, (long) cons(0, 0),
            LD, (long) cons(0, 1),
            LNIL,
            LD, (long) cons(1, 0), CONS,
            LD, (long) cons(1, 1), CONS, CDR, CAR,
            LDC, sym("hi"),
            LDC, sym("hi"),
            EQ);
    eval();
    listeq(LIST(sym("t"), I(4), I(2), I(1)), S);
}

void t9() {
    C = LIST(LDC, I(1), LDC, I(2), ADD,
            LDC, I(5), LDC, I(3), SUB,
            LDC, I(4), LDC, I(2), MUL,
            LDC, I(12), LDC, I(2), DIV,
            LDC, I(12), LDC, I(5), REM,
            LDC, I(8), LDC, I(32), LT);
    eval();
    listeq(LIST(sym("t"), I(2), I(6), I(8), I(2), I(3)), S);
}

/*void t9() {
    // (def (a)
    //  (a))
    C = LIST(DUM,
            LNIL,
            LDF, (long) LIST(LNIL, LD, (long) cons(1, 0), TAP),
            CONS,
            LDF, (long) LIST(LNIL, LD, (long) cons(0, 0), AP), RAP);
    eval();
    print_val((long) S);
}*/

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
CLEAN_TEST(t9)
END
