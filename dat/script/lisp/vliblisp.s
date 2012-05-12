( defun myDouble ( a ) 
	( + a a ))

( defun lisp_object_process ( ob func )
	(eval (append func ob)))
