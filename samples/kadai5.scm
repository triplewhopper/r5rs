(define (get-key t) (caar t))
(define (get-val t) (cdar t))
(define (get-kvpair t) (car t))
(define (get-left t) (cadr t))
(define (get-right t) (cddr t))

(define (new-node key val left right) 
  (cons (cons key val) 
        (cons left right)))

(define (update-left t new-left)
  (cons (car t) 
        (cons new-left 
              (get-right t))))

(define (update-right t new-right)
  (cons (car t)
        (cons (get-left t)
              new-right)))

(define (update-val t new-val)
  (cons (cons (get-key t) 
              new-val)
        (cdr t)))

(define (update-children t left right)
  (cons (car t) (cons left right)))

(define (delete-min t)
  (cond ((btree-null? (get-left t)) (cons (get-right t) t))
        (else (let* ((res (delete-min (get-left t))) 
                     (t.left (car res)) 
                     (min (cdr res)))
                    (cons (update-left t t.left)
                          min)))))

(define (delete-root t)
  (cond ((btree-null? (get-left t)) (get-right t))
        ((btree-null? (get-right t)) (get-left t))
        (else (let* ((tmp (delete-min (get-right t)))
                     (right (car tmp))
                     (min (cdr tmp)))
                    (update-children min (get-left t) right)))))

(define (btree-empty) 
  ;; 空木を返す。
  'btree-empty)

(define (new-leaf-node key val) 
  (new-node key val (btree-empty) (btree-empty)))

(define (btree-null? t)
  ;; 二分木`t'が空かどうかを真偽値で返す。
  (eq? t 'btree-empty)
  )
(define (btree-insert key val t)
  ;; 文字列`key'をキーとして文字列`val'を二分探索木`t'に挿入し、その木を返す。
  (cond ((btree-null? t) (new-leaf-node key val))
        ((string<? key (get-key t)) (update-left t (btree-insert key val (get-left t))))
        ((string=? key (get-key t)) (update-val t val))
        ((string>? key (get-key t)) (update-right t (btree-insert key val (get-right t))))
        (else (display "error in btree-insert\n"))))

(define (btree-delete key t)
  ;; 文字列`key'をキーとする項目を、二分探索木`t'から削除し、その木を返す。
  (cond ((btree-null? t) (btree-empty))
        ((string<? key (get-key t)) (update-left t (btree-delete key (get-left t))))
        ((string=? key (get-key t)) (delete-root t))
        ((string>? key (get-key t)) (update-right t (btree-delete key (get-right t))))))

(define (btree-search key t)
  ;; 文字列`key'をキーとして二分探索木`t'を検索し、キーとデータのペアを返す。
  ;; 見つからない場合は、#fを返す。
  (cond ((btree-null? t) #f)
        ((string<? key (get-key t)) (btree-search key (get-left t)))
        ((string=? key (get-key t)) (get-kvpair t))
        ((string>? key (get-key t)) (btree-search key (get-right t)))))

; (let ((t (btree-empty)))
;   (define (input->string x)
;     (cond
;      ((symbol? x) (symbol->string x))
;      ((number? x) (number->string x))
;      ((string? x) x)
;      (else #f)))
;   (define (main-loop t)
;     (let ((cmd (read)))
;       (cond
;        ((equal? cmd 'insert)
;         (let* ((key (input->string (read)))
;                (val (input->string (read))))
;           (main-loop (btree-insert key val t))))
;        ((equal? cmd 'delete)
;         (let* ((key (input->string (read))))
;           (main-loop (btree-delete key t))))
;        ((equal? cmd 'search)
;         (let* ((key (input->string (read)))
;                (entry (btree-search key t)))
;           (if (not entry)
;               (display "(not found)\n")
;               (begin
;                 (display (cdr entry))
;                 (newline)))
;           (main-loop t)))
;        ((or (equal? cmd 'quit) (eof-object? cmd))
;         #t)
;        (else
;         (display "(unknown command)\n")
;         (main-loop t)))))
;   (main-loop t))

