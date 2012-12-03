(defun transform ( elements )
	(foldl object_process (create_transform) elements))

( defun apply ( func arg )
	( func arg ))

( defun apply_reverse ( arg func )
	( func arg ))

( defmacro apply_attributes ( attributes object )
	( foldl apply_reverse object attributes ))

( defun add_model ( args )
	( apply_attributes args ( create_model )))

#( defmacro add_transform ( model args )
	#( apply_attributes args ( create_transform model )))

( defmacro add_particle ( transform args )
	( foldl args ( create_particle transform )))

( defmacro add_mesh ( model args )
	( foldl args ( create_mesh model )))

# create model
# add transform to model
# add particle to model
# add particle to model
#
# create transform
# add particle to transform
# add particle to transform
#
# create particle
# create particle

( defun add_transform ( args )
	( lambda ( model ) ( add_transform_to_model model ( apply_attributes args ( new_create_transform )))))
