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


(define s (make-monitored sqrt))
(s 100)
(s 'how-many-calls?)
(s 400)
(s 'how-many-calls?)
(s 'reset-count)
(s 900)
(s 'how-many-calls?)

