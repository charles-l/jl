#ifndef OPS_H
#define OPS_H

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
#endif // OPS_H
