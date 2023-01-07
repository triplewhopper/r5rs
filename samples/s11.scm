(define (list . xs) xs)

(define (foldr f z xs)
	(if (null? xs)
		z
		(f (car xs) 
		   (foldr f z (cdr xs)))))

(define (my-reverse xs)
	((foldr (lambda (b g) (lambda (x) (g (cons b x))))
	        (lambda (x) x)
	        xs)
	 '() ))

(my-reverse (list 1 2 3 4 5))