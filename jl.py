def createEnum(elems):
    for i, e in enumerate(elems):
        globals()[e] = i
        i += 1

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

class Type():
    _type = 0

    def __repr__(self):
        return "#<unknown>"

class Nil(Type):
    _type = T_NIL
    def __repr__(self):
        return "nil"

class Int(Type):
    _type = T_INT
    def __init__(self, val):
        self._val = val

    def __repr__(self):
        return str(self._val)

class Sym(Type):
    _type = T_SYM
    def __init__(self, val):
        # TODO: store value in a global hash table
        self._val = val

    def __repr__(self):
        return "'" + str(self._val)

class Cons(Type):
    _type = T_CONS
    def __init__(self, car, cdr):
        self.car = car
        self.cdr = cdr

    def __repr__(self):
        return "(" + str(self.car) + " . " + str(self.cdr) + ")"

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

# TODO: standardize stack direction for everything

def run(fp):
    C = [
            LNIL,
            LI, 9,
            LSYM, ord('h'), ord('a'), ord('h'), 0x0,
            CONS,
            CDR,
            LD, 0, 1,
            LNIL,
            SEL,
                LI, 3, JOIN,
                LD, 0, 0, JOIN,
            LSYM, ord('b'), ord('a'), ord('d'), 0x0
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
            s = popTo(C, indexOf(C, 0x0))
            C.pop(0) # get rid of str terminator
            S.append(Sym(s))
        elif b == LD:
            framei = C.pop(0)
            vari = C.pop(0)
            S.append(E[framei][vari])
        elif b == SEL:
            c = S.pop()
            t = popTo(C, indexOf(C, JOIN) + 1)
            f = popTo(C, indexOf(C, JOIN) + 1)
            D.append(C)
            if c != nil:
                C = t
            else:
                C = f
        elif b == JOIN:
            C = D.pop(0)
        elif b == LDF: # TODO: possibly store functions in a separate section of the bytecode?
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
    return S, E, C, D
