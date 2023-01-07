(define (my-gcd x y)
	(if (= 0 y) x (my-gcd y (remainder x y))))


(my-gcd 9801 1287)