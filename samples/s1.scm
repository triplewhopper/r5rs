(define (list . lst) lst)

(define (foldr f z xs)
	(if (null? xs)
		z
		(f (car xs) 
		   (foldr f z (cdr xs)))))
(define (my-min xs)
	(foldr (lambda (h t) 
				   (if (null? t) 
				   	   h 
				   	   (if (< h t) h t)))
			'()
			xs))

(define (min-of-four a1 a2 a3 a4)
	(my-min (list a1 a2 a3 a4)))

(min-of-four 3 5 4 6)