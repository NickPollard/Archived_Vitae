(model (object_process (meshLoadFile "dat/model/borehole.obj" )
			 (quote (attribute "diffuse_texture" "dat/img/borehole.tga")))
		(transform
			(quote ((attribute "translation" (vector 0.2 0.7 -2.1 1.0))
					(attribute "particle" (particleLoad "dat/script/lisp/borehole.s"))
)))
)
