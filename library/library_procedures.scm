(define (not obj)
    (if (eq? obj #f) #t #f))

(define (boolean? obj)
    (if (eq? obj #t)
        #t
        (if (eq? obj #f)
            #t
            #f)))

(define (caar obj) (car (car obj)))
(define (cadr obj) (car (cdr obj)))
(define (cdar obj) (cdr (car obj)))
(define (cddr obj) (cdr (cdr obj)))

(define (caaar obj) (car (car (car obj))))
(define (caadr obj) (car (car (cdr obj))))
(define (cadar obj) (car (cdr (car obj))))
(define (caddr obj) (car (cdr (cdr obj))))
(define (cdaar obj) (cdr (car (car obj))))
(define (cdadr obj) (cdr (car (cdr obj))))
(define (cddar obj) (cdr (cdr (car obj))))
(define (cdddr obj) (cdr (cdr (cdr obj))))

(define (caaaar obj) (car (car (car (car obj)))))
(define (caaadr obj) (car (car (car (cdr obj)))))
(define (caadar obj) (car (car (cdr (car obj)))))
(define (caaddr obj) (car (car (cdr (cdr obj)))))
(define (cadaar obj) (car (cdr (car (car obj)))))
(define (cadadr obj) (car (cdr (car (cdr obj)))))
(define (caddar obj) (car (cdr (cdr (car obj)))))
(define (cadddr obj) (car (cdr (cdr (cdr obj)))))

(define (cdaaar obj) (car (car (car (car obj)))))
(define (cdaadr obj) (car (car (car (cdr obj)))))
(define (cdadar obj) (car (car (cdr (car obj)))))
(define (cdaddr obj) (car (car (cdr (cdr obj)))))
(define (cddaar obj) (car (cdr (car (car obj)))))
(define (cddadr obj) (car (cdr (car (cdr obj)))))
(define (cdddar obj) (car (cdr (cdr (car obj)))))
(define (cddddr obj) (car (cdr (cdr (cdr obj)))))


(define (null? obj) (if (eq? '() obj) #t #f))

(define (list? obj)
    (if (null? obj)
        #t
        (if (pair? obj)
            (list? (cdr obj))
            #f)))

(define (list . args) args)

# define (length list)
