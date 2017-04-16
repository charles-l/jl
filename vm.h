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

long car(pair c) {
    assert((long) c & T_CONS);
    return c[0];
}

long cdr(pair c) {
    assert((long) c & T_CONS);
    return c[1];
}

void print_val(long v) {
    if((v & BITMASK) == T_INT) {
        printf("INT<%d>", v >> NSHIFT);
    //} else if ((v & BITMASK) == T_BOOL) {
        //printf((v >> 3) == 1 ? "#t" : "nil");
    } else if ((v & BITMASK) == T_CONS) {
        long *p = (long *) (v & ~T_CONS);
        if(p) {
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
    } /*else if ((v & BITMASK) == T_VEC) {
        long *vec = (long *) (v & ~T_VEC);
        unsigned short n = ((short *) vec)[0];
        printf("VEC<n: %d", n);
        int i = 0;
        for(; i < n && i < 5; i++) {
            putchar(' ');
            print_val(vec[i + 1]);
        }
        if (i < n) printf("...");
        printf(">");
    } */ else {
        printf("?<0x%x>", v);
    }
}

void print_utlist(pair l) {
    if(l) {
        putchar('(');
        print_val(car(l));
        printf(" . ");
        print_utlist((pair) cdr(l));
        putchar(')');
    } else {
        printf("'()");
    }
}
#endif // OPS_H
