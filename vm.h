#ifndef OPS_H
#define OPS_H

#define NSHIFT 3
#define NIL ((pair) 0x1)

#define BITMASK 0b111

typedef enum {
    T_INT = 0x0,
    T_CONS = 0x1,
    T_FUN = 0x2,
} type;

typedef struct {
    long  *frees; // frees vector
    void  *code;
    short nargs;
} closure;

typedef long *pair;

#define car_(c) ((c)[0])
#define cdr_(c) ((c)[1])

#define UNTAGC(c) ((pair) (((long) (c)) & ~T_CONS))
#define CHECK_TAG(t, v) ((((long) (v)) & BITMASK) == (t))
#define IS_CONS(v) CHECK_TAG(T_CONS, (v))
#define IS_INT(v) CHECK_TAG(T_INT, (v))
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
    //} else if ((v & BITMASK) == T_BOOL) {
        //printf((v >> 3) == 1 ? "#t" : "nil");
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
        closure *c = (closure *) (v & ~T_FUN);
        printf("FUN<code: %p nargs: %d frees: ", c->code, c->nargs);
        print_val((long) c->frees);
        putchar('>');
    } else {
        printf("?<0x%x>", v);
    }
}

void print_utlist(pair l) {
    if(l != NIL) {
        print_val(car_(l));
        if(cdr_(l) != (long) NIL) {
            putchar(' ');
            print_utlist((pair) cdr_(l));
        }
    } else {
        printf("'()");
    }
}
#endif // OPS_H
