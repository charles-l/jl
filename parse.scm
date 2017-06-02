(use srfi-1 matchable holes)

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
      (list-index (@> eq? o) ops))))

(define (add-to-env env new-vars)
  (cons new-vars env))

(define (ensure! x . msg)
  (if x
    x
    (error (apply conc msg))))

(define (lookup env name)
  (cons 0 (ensure! (list-index (@> eq? name) (car env))
                               "lookup failed to find variable " name)))

(define (parse e env)
  (match e
         (('if c t f)               `(,@(parse c env) SEL (,@(parse t env) JOIN) (,@(parse f env) JOIN)))
         (('lambda (a ...) b ...)   `(LDF ,@(map (@< parse (add-to-env env a)) b)))
         (('quote s)                `(LDC ,s))
         (('cons a b)               `(,@(parse b env) ,@(parse a env) CONS))
         (('+ a b)                  `(,@(parse b env) ,@(parse a env) ADD))
         (('- a b)                  `(,@(parse b env) ,@(parse a env) SUB))
         (('* a b)                  `(,@(parse b env) ,@(parse a env) MUL))
         (('/ a b)                  `(,@(parse b env) ,@(parse a env) DIV))
         (('< a b)                  `(,@(parse b env) ,@(parse a env) LT))
         ((f a ...)                 `(,@(map (@< parse env) a) ,@(parse f env) AP))
         ((? symbol? s)             `(LD ,(lookup env s)))
         ((? integer? i)            `(LDC ,(arithmetic-shift i 3)))))

(parse ''(1 2 3) '())

(parse ''blah '())

(parse '(+ (* 1 1) (+ (- 2 1) (/ 3 2))) '())
(parse '(< 1 2) '())
(parse '(if 1 1 2) '())

(parse '(lambda (x y) (+ 1 x)) '())

(parse '((lambda (a b c) (- (+ a b) c)) 1 2 3) '())

(parse '(t 1 2 3) '((t)))
