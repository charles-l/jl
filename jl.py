def createEnum(elems):
    for i, e in enumerate(elems):
        globals()[e] = i
        i += 1

TERM=0 # string/list bytecode terminator

createEnum(["T_NIL", "T_INT", "T_CONS", "T_FUN", "T_SYM"])

createEnum([
    "LNIL", "LI", "LSYM", # load nil, int, symbol
    "LD",                 # load from env
    "SEL",
    "JOIN",
    "LDF",
    "AP",
    "TAP",
    "RAP",
    "PUSHE",
    "RET",
    "DUM",
    "CAR",
    "CDR",
    "ATOM",
    "CONS",
    "EQ",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "REM",
    "LT",
    "CAP",
    ])

def debug(msg):
    print "debug: ", msg

def entry_point(argv):
    debug(run(0))
    return 0

def target(*args):
    return entry_point

class Nil(): # bottom level type
    _type = T_NIL
    def __repr__(self):
        return "nil"

class Int(Nil):
    _type = T_INT
    def __init__(self, val):
        self._val = val

    def __repr__(self):
        return str(self._val)

class Sym(Nil):
    _type = T_SYM
    def __init__(self, val):
        # TODO: store value in a global hash table
        self._val = val

    def __repr__(self):
        return "'" + str(bytearray(self._val))

class Cons(Nil):
    _type = T_CONS
    def __init__(self, car, cdr):
        self.car = car
        self.cdr = cdr

    def __repr__(self):
        return "(" + str(self.car) + " . " + str(self.cdr) + ")"

class Closure(Cons):
    _type = T_FUN
    def __init__(self, body, env):
        self.car = body
        self.cdr = env

    def __repr__(self):
        return "<closure>"

nil = Nil()

def indexOf(a, v):
    for i, e in enumerate(a):
        if e == v:
            return i
    return 0

def popTo(a, i):
    r = a[:i]
    del a[:i]
    return r

def popList(a, terminator=TERM):
    r = popTo(a, indexOf(a, terminator))
    if terminator == TERM:
        a.pop(0) # throw out terminator
    else:
        r.append(a.pop(0)) # append terminating instr
    return r

# TODO: standardize stack direction for everything

def run(fp):
    C = [
            LNIL,
            LI, 9,
            LSYM, ord('h'), ord('a'), ord('h'), TERM,
            CONS,
            CDR,
            LD, 0, 1,
            LNIL,
            SEL,
                LI, 3, JOIN,
                LD, 0, 0, JOIN,
            LSYM, ord('b'), ord('a'), ord('d'), TERM,
            LDF, 5, LSYM, ord('y'), ord('o'), TERM, RET,
            AP
            ]
    S = []
    E = [[Int(1), Int(2)]]
    D = []
    while len(C) > 0:
        b = C.pop(0)
        if b == LNIL:
            S.append(nil)
        elif b == LI:
            S.append(Int(C.pop(0)))
        elif b == LSYM:
            s = popList(C)
            S.append(Sym(s))
        elif b == LD:
            framei = C.pop(0)
            vari = C.pop(0)
            S.append(E[framei][vari])
        elif b == SEL:
            cond = S.pop()
            tru = popList(C, JOIN)
            fals = popList(C, JOIN)
            D.append(C)
            if cond != nil:
                C = tru
            else:
                C = fals
        elif b == JOIN:
            C = D.pop(0)
        elif b == LDF:
            finstrs = C.pop(0)
            fbody = popTo(C, finstrs)
            S.append(Closure(fbody, E))
            pass
        elif b == AP:
            closure = S.pop()
            args = S.pop()
            D.append(S)
            D.append(E)
            D.append(C)
            S = []
            E = Cons(args, closure.cdr)
            C = closure.car
        elif b == TAP:
            pass
        elif b == RAP:
            pass
        elif b == PUSHE:
            pass
        elif b == RET:
            r = S.pop()
            C = D.pop()
            E = D.pop()
            S = D.pop()
            S.append(r)
        elif b == DUM:
            pass
        elif b == CONS:
            cdr = S.pop()
            car = S.pop()
            S.append(Cons(car, cdr))
        elif b == CAR:
            c = S.pop()
            S.append(c.car)
        elif b == CDR:
            c = S.pop()
            S.append(c.cdr)
        elif b == ATOM:
            pass
        elif b == ADD:
            pass
        elif b == SUB:
            pass
        elif b == MUL:
            pass
        elif b == DIV:
            pass
        elif b == REM:
            pass
        elif b == LT:
            pass
    return S, E, C, D

entry_point(nil)
