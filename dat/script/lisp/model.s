(defun transform ( elements )
	(foldl object_process (create_transform) elements))
