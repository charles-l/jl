#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "vm.h"

#define STACK_SIZE 1024
#define STACK_END (stack + STACK_SIZE - 1)
#define LOCALS_SIZE 16
#define ADV_I (*ip++)
#define CUR_I (*ip)

typedef enum {
    I_EOS, // end of instruction stream
    I_PUSH,
    I_POP,
    I_DUP, // duplicate ith element on the stack and push it to the top
    I_JMP, // ip += stack[1]
    I_CONS, // cons stack[1] stack[2]
    I_CMPNIL, // cmpnil stack[0]
    I_JMPNIL, // jmp if stack[1] is nil
    I_CAR, // car stack[1]
    I_CDR, // cdr stack[1]

    I_VECNEW, // vec new of size stack[0]
    I_VECREF, // get val at index (stack[1]) of vec (stack[2])
    I_VECSET, // set val (stack[1]) at index (stack[2]) of vec (stack[3])
    I_VECLEN, // get the vector length

    I_CLOSNEW, // new closure: stack[3] = code pointer, stack[2] = frees vec, stack[1] = nargs
    I_RET, // return from function

    I_CALL, // execute lambda
    I_CCALL, // call c function
} op;

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

long untag_int(long v) {
    assert((v & BITMASK) == INT);
    return v >> 3;
}

long bool_new(long v) {
    return (v << NSHIFT) | BOOL;
}

long vec_new(unsigned short n) {
    // upper two bytes are the max length
    // lower two bytes unused for now.
    // the rest are long slots that can be filled with values
    void *vec = malloc((n + 1) * sizeof(long));
    ((short *) vec)[0] = n;
    memset(((long *) vec) + 1, 0, sizeof(long) * n);
    return ((long) vec | VEC);
}

long vec_ref(void *vec, unsigned short i) {
    assert(i < ((short *) vec)[0]);
    return(((long *) vec)[i + 1]);
}

long vec_set(void *vec, unsigned short i, long val) {
    assert(i < ((short *) vec)[0]); // sanity check on index
    ((long *) vec)[i + 1] = val;
    return((long) vec | VEC); // should we really push back the vector?
    // should vectors be immutable?
}

long eval(long *code, long **spp) {
    long *sp = *spp;
    long *ip = code; // instruction pointer
    while(CUR_I != I_EOS) {
        switch(ADV_I) {
            case I_PUSH:
                PUSH(ADV_I);
                break;
            case I_POP:
                POP();
                break;
            case I_DUP:
                PUSH(sp[ADV_I + 1]);
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
                        ip += (int) CUR_I;
                    } else {
                        ADV_I;
                    }
                }
                break;
            case I_CAR:
                {
                    long v = POP();
                    assert((v & BITMASK) == CONS);
                    PUSH(CAR(v & ~CONS));
                }
                break;
            case I_CDR:
                {
                    long v = POP();
                    assert((v & BITMASK) == CONS);
                    PUSH(CDR(v & ~CONS));
                }
                break;
            case I_VECNEW:
                {
                    unsigned short n = untag_int(POP());
                    PUSH(vec_new(n));
                }
                break;
            case I_VECREF:
                {
                    unsigned short i = untag_int(POP());
                    void *vec = (void *) (((long) POP()) & ~VEC);
                    PUSH(vec_ref(vec, i));
                }
                break;
            case I_VECSET:
                {
                    long val = POP();
                    unsigned short i = untag_int(POP());
                    void *vec = (void *) (((long) POP()) & ~VEC);
                    PUSH(vec_set(vec, i, val));
                }
                break;
            case I_VECLEN:
                {
                    short* vec = ((short *) (POP() & ~VEC));
                    PUSH(int_new(vec[0]));
                }
                break;
            case I_CLOSNEW:
                {
                    closure *c = malloc(sizeof(closure));
                    c->nargs = POP();
                    c->frees = (long *) POP();
                    c->code = (void *) POP();
                    // TODO: assert types
                    PUSH((long) c | FUN);
                }
                break;
            case I_CALL:
                {
                    long *c = (long *) POP();
                    PUSH(eval(c, spp));
                }
                break;
            case I_RET:
                {
                    return POP();
                }
                break;
        }
    }
    *spp = sp;
    return NIL;
}

int main() {
    long stack[STACK_SIZE];
    long locals[LOCALS_SIZE];
    long *sp = STACK_END;

    op test_fun[] = {
        I_PUSH, int_new(8),
        I_PUSH, int_new(8),
        I_RET,
    };

    // TODO: convert this to bytes so its
    // ya know, bytecode?
    long in_stream[] = {
        I_PUSH, int_new(4),
        I_PUSH, bool_new(1),
        I_PUSH, int_new(6),
        I_POP,
        I_DUP, 2,
        I_PUSH, int_new(6),
        I_JMP, 2, // skip next instruction
        I_PUSH, int_new(0xDEADBEEF << 1),
        I_PUSH, bool_new(0),
        I_PUSH, int_new(94),
        I_CONS,
        I_DUP, 1, // dup cons on the top of the stack
        I_CAR,
        I_DUP, 2,
        I_CDR,
        I_PUSH, bool_new(0),
        I_CMPNIL,

        I_PUSH, int_new(1337),

        ///

        I_PUSH, bool_new(1),
        I_JMPNIL, 5,

        I_PUSH, int_new(1),
        I_JMP, 3,

        I_PUSH, int_new(2),

        I_PUSH, int_new(3),
        I_VECNEW,

        I_PUSH, int_new(2),
        I_PUSH, bool_new(1),
        I_VECSET,

        I_PUSH, int_new(1),
        I_PUSH, int_new(8),
        I_VECSET,

        I_DUP, 1,
        I_VECLEN,

        I_PUSH, (long) test_fun,
        I_PUSH, int_new(2),
        I_VECNEW,
        I_PUSH, 1,
        I_CLOSNEW,

        I_CALL,

        I_EOS,
    };

    eval(in_stream, &sp);
    dump_stack(stack, sp);
}
