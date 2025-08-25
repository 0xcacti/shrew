(define fiboncci (lambda (n)
    (if (or (= n 0) (= n 1))
        n
        (+ (fiboncci (- n 1)) (fiboncci (- n 2))))))

(fiboncci 10)
