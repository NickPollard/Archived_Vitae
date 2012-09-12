# Canyon zones
(defun canyon_zone ( properties )
	(foldl object_process (canyon_zone_create) properties))
