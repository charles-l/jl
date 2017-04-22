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

long car(pair c) {
    assert((long) c & T_CONS);
    return car_((pair) ((long) c & ~T_CONS));
}

long cdr(pair c) {
    assert((long) c & T_CONS);
    return cdr_((pair) ((long) c & ~T_CONS));
}

void print_val(long v) {
    if((v & BITMASK) == T_INT) {
        printf("INT<%d>", v >> NSHIFT);
    //} else if ((v & BITMASK) == T_BOOL) {
        //printf((v >> 3) == 1 ? "#t" : "nil");
    } else if ((v & BITMASK) == T_CONS) {
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
    } else if ((v & BITMASK) == T_FUN) {
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
