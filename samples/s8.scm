(define (compose f g) (lambda (x) (g (f x))))

((compose (lambda (x) (+ x 1)) (lambda (x) (* x x))) 2)