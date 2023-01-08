(define (complex a b) (cons a b)) ; aを実数部、bを虚数部とする複素数を返す。
(define (complex= c1 c2) (equal? c1 c2)) ; 複素数`c1'と`c2'が等しいかどうかをブール値で返す。
(define (complex+ c1 c2) ; 複素数`c1'と`c2'の和を返す。
	(cons (+ (car c1) (car c2)) 
		  (+ (cdr c1) (cdr c2)))) 
(define (complex* c1 c2) ; 複素数`c1'と`c2'の積を返す。
	(cons (- (* (car c1) (car c2))
			 (* (cdr c1) (cdr c2)))
		  (+ (* (car c1) (cdr c2))
		  	 (* (cdr c1) (car c2)))))

(complex 1 2)
(complex* (complex 1/2 2/3)
(complex 5 -3/2))