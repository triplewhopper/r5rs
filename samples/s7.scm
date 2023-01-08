(define (abs x) (if (< x 0) (- x) x))
(define (sqrt-base x) (lambda (t) (- (* t t) x)))
(define (deriv f dx) ; deriv :: (Num->Num)->Num->(Num->Num)
	(lambda (x) 
		(/ (- (f (+ x dx)) 
			  (f x)) 
		   dx)))

(define (newton-iter g d guess)
	(if (good-enough? g guess)
	    guess
		(newton-iter g d (improve g d guess))))

(define (good-enough? g guess) ; good-enough? :: ( Num -> Num ) -> Num -> Bool
	(< (abs (g guess)) 
	   1e-7))

(define (improve g d guess) ; improve (Num->Num)->(Num->Num)->Num->Num
	(- guess 
	   (/ (g guess) 
	   	  (d guess))))

(define (improve2 g guess) ; improve (Num->Num)->(Num->Num)->Num->Num
	(- guess 
	   (/ (g guess) 
	   	  ((deriv g 1e-4) guess))))

(define (newton-iter2 g guess)
	(if (good-enough? g guess)
	    guess
		(newton-iter2 g (improve2 g guess))))

(define (square x) (* x x))
((deriv square 0.0001) 3)
(define (sqrt3 x) (newton-iter2 (sqrt-base x) 1.0))
(sqrt3 2)
