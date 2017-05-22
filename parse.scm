(use srfi-1)

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

; TODO: implement a parser
