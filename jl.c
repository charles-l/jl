#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ops.h"

#define STACK_SIZE 1024
#define STACK_END (stack + STACK_SIZE - 1)
#define LOCALS_SIZE 16

void print_val(long v) {
    if((v & BITMASK) == INT) {
        printf("INT<%d>", v >> NSHIFT);
    } else if ((v & BITMASK) == BOOL) {
        printf((v >> 3) == 1 ? "#t" : "nil");
    } else if ((v & BITMASK) == CONS) {
        long *p = (long *) (v & ~CONS);
        printf("(");
        print_val(CAR(p));
        printf(" . ");
        print_val(CDR(p));
        printf(")");
    } else if ((v & BITMASK) == FUN) {
        func *f = (func *) (v & ~FUN);
        printf("FUN<args: %d frees: %d>", f->nargs, f->nfrees);
    } else {
        printf("?<0x%x>", v);
    }
}

void dump_stack(long *stack, long *sp) {
    puts("=== TOP ===");
    while(sp++ < STACK_END) {
        printf("0x%x\t%016p", sp, *sp);
        putchar('\t');
        print_val(*sp);
        putchar('\n');
    }
    puts("=== BOTTOM ===");
}

#define PUSH(v) (*sp-- = (v))
#define POP() ((sp++)[1])

long int_new(long v) {
    return v << NSHIFT;
}

long bool_new(long v) {
    return (v << NSHIFT) | BOOL;
}

long cons_new(long a, long b) {
    long *v = malloc(sizeof(long) * 2);
    v[0] = a;
    v[1] = b;
    return (long) v | CONS;
}

long fun_new(short nfrees, short nargs, void *code) {
    func *f = malloc(sizeof(func));
    f->frees = malloc(sizeof(long) * nfrees);
    f->code = code;
    f->nargs = nargs;
    f->nfrees = nfrees;
    return (long) f | FUN;
}

int main() {
    long stack[STACK_SIZE];
    long locals[LOCALS_SIZE];
    long *sp = STACK_END;

    op in_stream[] = {
        I_PUSH, int_new(4),
        I_PUSH, bool_new(1),
        I_PUSH, int_new(6),
        I_POP,
        I_DUP, 2,
        I_PUSH, int_new(6),
        I_JMP, 2, // skip next instruction
        I_PUSH, int_new(0xDEADBEEF << 1),
        I_PUSH, bool_new(0),
        I_PUSH, int_new(2),
        I_CONS,
        I_PUSH, bool_new(0),
        I_CMPNIL,

        I_PUSH, int_new(1337),
        ///

        I_PUSH, bool_new(1),

        I_JMPNIL, 5,

        I_PUSH, int_new(1),

        I_JMP, 3,
        I_PUSH, int_new(2),

        I_EOS,
    };
    op *ip = in_stream; // instruction pointer

    while(*ip) {
        switch(*ip++) {
            case I_PUSH:
                PUSH(*ip++);
                break;
            case I_POP:
                POP();
                break;
            case I_DUP:
                PUSH(sp[*ip++ + 1]);
                break;
            case I_JMP:
                ip += (int) ip[0];
                break;
            case I_CONS:
                PUSH(cons_new(sp[2], sp[3]));
                break;
            case I_CMPNIL:
                {
                    long v = POP();
                    assert((v & BITMASK) == BOOL);
                    PUSH(bool_new(v == NIL));
                }
                break;
            case I_JMPNIL:
                {
                    long v = POP();
                    assert((v & BITMASK) == BOOL);
                    if(v == NIL) {
                        ip += (int) *ip;
                    } else {
                        ip++;
                    }
                }
                break;
        }
    }

    dump_stack(stack, sp);
}
