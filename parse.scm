(use srfi-1 matchable)

(define op
  (let ((ops '(LNIL
                LDC
                LD
                SEL
                JOIN
                LDF
                AP
                TAP
                RAP
                RET
                DUM
                CAR
                CDR
                ATOM
                CONS
                EQ
                ADD
                SUB
                MUL
                DIV
                REM
                LT
                CAP)))
    (lambda (o)
      (list-index (cut eq? o <>) ops))))

(define (parse e)
  (match e
         (('if c t f)            `(,@(parse c) SEL (,@(parse t) JOIN) (,@(parse f) JOIN)))
         (('+ a b)                `(,@(parse b) ,@(parse a) ADD))
         (('- a b)                `(,@(parse b) ,@(parse a) SUB))
         (('* a b)                `(,@(parse b) ,@(parse a) MUL))
         (('/ a b)                `(,@(parse b) ,@(parse a) DIV))
         (('< a b)                `(,@(parse b) ,@(parse a) LT))
         ((? integer? i)         `(LDC ,(arithmetic-shift i 3)))
         ((? symbol? s)          `(LDC ,(symbol->string s)))
         ((? pair? i)            `(,@(parse (cdr i)) ,@(parse (car i)) CONS))
         (()                     `(LNIL))))

(parse '(1 2 3))
(parse 'blah)
(parse '(if 1 1 2))
(parse '(+ (* 1 1) (+ (- 2 1) (/ 3 2))))
(parse '(< 1 2))
