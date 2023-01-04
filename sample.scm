(define (list . x) x)
(define (begin-impl x) (if (null? (cdr x)) (car x) (begin-impl (cdr x))))
(define (begin . x) (begin-impl x))
(define (make-monitored f)
	(define x 0)
	(lambda (arg)
		(if (eq? arg 'how-many-calls?)
		    x
			(if (eq? arg 'reset-count)
			    (set! x 0)
			    (begin (set! x (+ x 1)) (f arg))))))

(define (abs x) (if (> x 0) x (- x)))
(define (sqrt-next xn a) (* 0.5 (+ xn (/ a xn))))
(define (sqrt-judge xn xn+1 a) (if (< (abs (- xn xn+1)) 1e-6) xn (sqrt-impl xn+1 a)))
(define (sqrt-impl xn a) (sqrt-judge xn (sqrt-next xn a) a))
(define (sqrt a) (if (>= 0 a) a (sqrt-impl (/ a 2) a)))

(define s (make-monitored sqrt))
(list (s 100) (s 400) (s 'how-many-calls?) (s 'reset-count) (s 900) (s 'how-many-calls?))

