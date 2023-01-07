(define (abs x) (if (> x 0) x (- x)))

(define <eps> 1e-6)
(define (sqrt a)
    (define (sqrt-next xn a) (* 0.5 (+ xn (/ a xn))))
    (define (sqrt-judge xn xn+1 a) (if (< (abs (- xn xn+1)) <eps>) xn (sqrt-impl xn+1 a)))
    (define (sqrt-impl xn a) (sqrt-judge xn (sqrt-next xn a) a))
(if (>= 0 a) a (sqrt-impl (/ a 2) a)))
