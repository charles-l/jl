#ifndef OPS_H
#define OPS_H

#define NSHIFT 3
#define NIL 0x1

#define BITMASK 0b111

typedef enum {
    INT = 0x0,
    BOOL = 0x1,
    CONS = 0x2,
    VEC = 0x3,
    FUN = 0x4,
} type;

#define CAR(c) (((long *) (c))[0])
#define CDR(c) (((long *) (c))[1])

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

    I_CALL, // call lambda
    I_CCALL, // call c function
} op;

#endif // OPS_H
