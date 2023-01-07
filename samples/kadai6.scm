(define (fib-impl n a b)
	(cond 
		((= 1 n) b)
		(else (fib-impl (- n 1) b (+ a b)))))

(define (fib2 n) 
	(cond 
		((<= n 1) n)
		(else (fib-impl n 0 1))))