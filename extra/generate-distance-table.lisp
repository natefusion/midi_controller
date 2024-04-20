(defun B (x)
  "Magnetic field in Gauss as a function of distance in millimeters for the magnet that we are using on the keyboard."
  (let* ((mm (expt 10 -3))
         (G 1)
         (Br (* 12100 G))
         (r (* 3 mm))
         (l (* 2.5 mm)))
    (flet ((sq (x) (expt x 2)))
          (* (/ Br 2)
             (- (/ (+ (* x mm) l)
                   (sqrt (+ (sq r) (sq (+ l (* x mm))))))
                (/ (* x mm)
                   (sqrt (+ (sq r) (sq (* x mm))))))))))

(defun hall-effect-response (B &optional (q 2.5) (s 1.6) (m 1000))
  "Voltage in adc bits vs the magnetic field in Gauss."
  (* (/ 1024 5) (+ q (* (/ s m) B))))

(defun collect-data (q s m)
  "Returns a list of adc value and gauss (at that adc value) pairs."
  (let* ((zero-response (hall-effect-response 0 q s m))
         (magnetic-field (B 0))
         (response (hall-effect-response magnetic-field q s m))
         (max-magnetic-field-reading (hall-effect-response m q s m))
         (result nil))
    (do ((x 0.01 (+ x 0.005)))
        ((<= response zero-response) result)
      (setf magnetic-field (B x)
            response (hall-effect-response magnetic-field q s m))
      (when (<= response max-magnetic-field-reading)
        (push (cons (truncate response) x) result)))))

(defun process-data (result)
  "Receives data from collect-data. Any gauss values with identical adc values are averaged."
  (loop with current-adc-value = (caar result)
        with sum = 0
        with n = 0
        with data = nil
        for (adc-value . distance-in-mm) in result
        do (if (= adc-value current-adc-value)
               (progn (incf sum distance-in-mm)
                      (incf n))
               (progn (push (/ sum n) data)
                      (setf current-adc-value adc-value
                            sum distance-in-mm
                            n 1)))
        finally (push (/ sum n) data)
                (return (reverse data))))

(defun collect-and-process-data (q s m)
  (process-data (collect-data q s m)))

(defun power-regression (data)
  (let ((sum-x 0)
        (sum-y 0)
        (sum-xy 0)
        (sum-xx 0)
        (sum-yy 0)
        (n (length data)))
    (loop for y in (mapcar (lambda (x) (log x)) data)
          for xx from 1
          for x = (log xx)
          do (incf sum-x x)
             (incf sum-y y)
             (incf sum-xy (* x y))
             (incf sum-xx (* x x))
             (incf sum-yy (* y y))
          finally (let* ((slope (/ (- (* n sum-xy) (* sum-x sum-y)) (- (* n sum-xx) (* sum-x sum-x))))
                         (intercept (exp (/ (- sum-y (* slope sum-x)) n)))
                         (r2 (expt (/ (- (* n sum-xy) (* sum-x sum-y)) (sqrt (* (- (* n sum-xx) (* sum-x sum-x)) (- (* n sum-yy) (* sum-y sum-y))))) 2)))
                    (format t "slope: ~a~%intercept: ~a~%r2: ~a~%" slope intercept r2)
                    (format t "~af * powf(index, ~af)~%~%" intercept slope)
                    (return (lambda (x) (* intercept (expt x slope))))))))

(defun generate-magic-numbers-for-all-sensors ()
  (loop for sensor in (list (list 2.595 1.700 1000)  ; s0
                            (list 2.525 1.765 1000)  ; s1
                            (list 2.570 1.725 1000)  ; s2 
                            (list 2.525 1.760 1000)  ; s3
                            (list 2.615 1.685 1000)) ; s4
        do (power-regression (cdr (apply #'collect-and-process-data sensor)))))
