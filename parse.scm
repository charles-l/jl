;; scheme impl: chicken scheme

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
                PUSHE
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

(define (add-to-frame env new-var)
  `((,new-var ,@(car env)) ,@(cdr env))) ; FIXME: don't push, append to end

(define (ensure! x . msg)
  (if x
    x
    (error (apply conc msg))))

(define (lookup env name)
  (call/cc (lambda (ret)
             (for-each (lambda (fr i)
                         (let ((li (list-index (@> eq? name) fr)))
                           (when li
                             (ret (cons i li)))))
                       env (iota (length env)))
             (error "failed to find variable " name))))

(define (parse-fun f a body env)
  (define (AP->TAP e) ; make call recursive apply
    (match e
           ((a ... (1 . 0) AP) `(,@a (1 . 0) TAP))
           (=> e)))

  (define (cur-fname? x) (eq? f x))
  (define does-recurse? (and f (find (match-lambda
                                       (((? cur-fname? n) _) #t)
                                       (=> #f))
                                     body)))

  (let ((byte-body `(LDF (,@(append-map (@@ ((if does-recurse? AP->TAP identity)
                                             (parse !!
                                                    (add-to-env
                                                      (if f (add-to-frame env f) env) a))))
                                        body) ,(if does-recurse? "" 'RET))))
        (pre/post (if does-recurse?
                    (@@ `(DUM LNIL ,@(AP->TAP !!) CONS
                              LDF (LNIL LD (0 . 0) AP) RAP))
                    identity)))
    (pre/post byte-body)))

(define (parse e env)
  (match e
         (('if c t f)               `(,@(parse c env) SEL (,@(parse t env) JOIN) (,@(parse f env) JOIN)))
         (('lambda (a ...) b ...)   (parse-fun #f a b env))
         (('quote s)                `(LDC ,s))
         (('cons a b)               `(,@(parse b env) ,@(parse a env) CONS))
         (('def (? symbol? a) b)    (set! env (add-to-frame env a)) ; AHHH!! MUTATION!!
                                    `(,@(parse b env) PUSHE))
         (('def ((? symbol? f) (? symbol? a) ...)
           b ...)                   (set! env (add-to-frame env f)) ; NOUUUUUU!!
                                    `(,@(parse-fun f a b env) PUSHE))
         (('+ a b)                  `(,@(parse b env) ,@(parse a env) ADD))
         (('- a b)                  `(,@(parse b env) ,@(parse a env) SUB))
         (('* a b)                  `(,@(parse b env) ,@(parse a env) MUL))
         (('/ a b)                  `(,@(parse b env) ,@(parse a env) DIV))
         (('< a b)                  `(,@(parse b env) ,@(parse a env) LT))
         ((f a ...)                 `(,@(append-map (@< parse env) a) ,@(parse f env) AP))
         ((? symbol? s)             `(LD ,(lookup env s)))
         ((? integer? i)            `(LDC ,(arithmetic-shift i 3)))))

(define (empty-env) '(()))

(parse ''(1 2 3) (empty-env))

(parse ''blah (empty-env))

(parse '(+ (* 1 1) (+ (- 2 1) (/ 3 2))) (empty-env))
(parse '(< 1 2) (empty-env))
(parse '(if 1 1 2) (empty-env))

(parse '(lambda (x y) (+ 1 x)) (empty-env))

(parse '((lambda (a b c) (- (+ a b) c)) 1 2 3) (empty-env))

(parse '(t 1 2 3) '((t)))

(parse '(def a 'b) (empty-env))

(parse '(def (x a) a) (empty-env))

(parse '(def (main)
             (def (x a) a)
             (x 1)) (empty-env))
