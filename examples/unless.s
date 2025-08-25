(defmacro unless (cond then else)
  `(if (not ,cond)
       ,then
       ,else))

(unless (= 1 2)
    (print "1 is not equal to 2")
    (print "1 is equal to 2"))
