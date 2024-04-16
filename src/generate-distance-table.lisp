(defun B (x)
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

(defun collect-data (q s m)
  (let* ((zero-response (hall-effect-response 0 q s m))
         (magnetic-field (B 0))
         (response (hall-effect-response magnetic-field q s m))
         (max-magnetic-field-reading (hall-effect-response m q s m))
         (result nil))
    (do ((x 0.01 (+ x 0.01)))
        ((<= response zero-response) result)
      (setf magnetic-field (B x)
            response (hall-effect-response magnetic-field q s m))
      (when (<= response max-magnetic-field-reading)
        (push (cons (truncate response) x) result)))))

(defun hall-effect-response (B &optional (q 2.5) (s 1.6) (m 1000))
  (* (/ 1024 5) (+ q (* (/ s m) B))))

(defun collect-hall-effect-data (&optional (q 2.5) (s 1.6) (m 1000))
  (flet ((process-data (result)
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
                         (return (reverse data)))))
    (format t "float distance_in_mm[] = {~%    ~{~a~^,~}~%};" (process-data (collect-data q s m)))))
