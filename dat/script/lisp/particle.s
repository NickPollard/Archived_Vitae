# ( particle 
#	((size (property ((0.0 1.0)
#					(1.0 0.0))))
#	(color (property ((0.0 1.0 1.0 1.0 0.0)
#					(1.0 0.0 1.0 0.0 1.0))))))
(defun particle_emitter_definition ( properties )
	(foldl object_process (particle_emitter_definition_create) properties))

# ( property ((0.0 1.0)
#			(1.0 0.0)))

(defun property (keys)
	(foldl property_addKey
		(property_create 
			(length (head keys)))
		 keys))
