(define (and . x)
    (define (and-impl x xs)
        (if (null? xs) x
            (if x (and-impl (car xs) (cdr xs))
                #f)))
    (if (null? x) #t (and-impl (car x) (cdr x))))

(define (or . x)
    (define (or-impl x xs)
        (if (null? xs) x
            (if x x
                (or-impl (car xs) (cdr xs)))))
    (if (null? x) #f (or-impl (car x) (cdr s))))

(define (not x) (if x #f #t))

; (assert (and (= 2 2) (> 2 1)) #t)
; (assert (and (= 2 2) (< 2 1)) #f)
; (assert (and 1 2 'c '("jjj" f g)) '("jjj" f g))
; (assert (and) #t)

