r5rs -l kadai1.scm -c "(min-of-four 3 5 4 6)" --assert 3

r5rs -l kadai2.scm -c "(even<odd? 7 1 2 4 9)" --assert "#t"

r5rs -l kadai3.scm -c "(my-gcd 9801 1287)" --assert 99

r5rs -l kadai4.scm -c "(fib 11)" --assert 89

r5rs -l kadai6.scm -c "(fib2 44)" --assert 701408733

r5rs -l kadai7.scm -c \
"(define (square x) (* x x))"\
"((deriv square 0.0001) 3)" --assert 6.000100000012054\
-c "(define (sqrt3 x)"\
"(newton-iter2 (sqrt-base x) 1.0))"\
"(sqrt3 2)" --assert 1.4142135624530596

r5rs -l kadai8.scm -c "((compose (lambda (x) (+ x 1)) (lambda (x) (* x x))) 2)" --assert 9

r5rs -l kadai9.scm -c "(f1 '(1 2 (3 4) 5))" "(f2 '((3)))" "(f3 '(1 (2 (4 (5 (6 (7 3)))))))" --assert 3 3 3

r5rs -l kadai10.scm -c "(sum-total (list 1 2 3 5))" --assert 11

r5rs -l kadai11.scm -c "(my-reverse (list 1 2 3 4 5))" --assert "(5 4 3 2 1)"

r5rs -l kadai12.scm -c "(define alist '((1 . \"One\") (2 . \"Two\") (3 . \"Three\")))"\
"(my-assoc 2 alist)" "(my-assoc 4 alist)" --assert "(2 . \"Two\")" "#f"

r5rs -l kadai13.scm -c "(my-reverse2 '(1 2 3 4 5))" --assert "(5 4 3 2 1)"

r5rs -l kadai14.scm -c "(complex 1 2)" "(complex* (complex 1/2 2/3) (complex 5 -3/2))" --assert "(1 . 2)" "(7/2 . 31/12)"

r5rs -l kadai18.scm -assert 10 

r5rs -l kadai20.scm -c "(define s (make-monitored sqrt))"\
"(s 100)" "(s 'how-many-calls?)" "(s 400)"\
"(s 'how-many-calls?)" "(s 'reset-count)" "(s 900)" "(s 'how-many-calls?)"\
--assert 10 1 20 2 30 1