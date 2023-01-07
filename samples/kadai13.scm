(define (my-fold op init l)
  (if (null? l)
      init
      (my-fold op (op (car l) init) (cdr l))))

(define (my-reverse2 xs)
  (my-fold (lambda (h t) (cons h t)) '() xs))