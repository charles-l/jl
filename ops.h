#ifndef OPS_H
#define OPS_H

#define NSHIFT 3
#define NIL 0x1

#define BITMASK 0b111

typedef enum {
    INT = 0x0,
    BOOL = 0x1,
    CONS = 0x2,
    FUN = 0x3,
} type;

typedef struct {
    long *frees;
    void *code;
    short nargs;
    short nfrees;
} func;

#define CAR(c) (((long *) c)[0])
#define CDR(c) (((long *) c)[1])

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
    I_VECREF, // vec ref of stack[1] at index stack[0]
    I_LNEW, // new lambda

    I_CALL, // call lambda
    I_CCALL, // call c function
} op;

#endif // OPS_H
