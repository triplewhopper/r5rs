(define (list . xs) xs)

(define (foldr f z xs)
	(if (null? xs)
		z
		(f (car xs) 
		   (foldr f z (cdr xs)))))

(define (sum-total xs)
	(foldr (lambda (h t) (+ h t)) 
		   0 
		   xs))

(sum-total (list 1 2 3 5))
