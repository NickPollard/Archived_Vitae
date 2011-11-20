(def map (args func)
	(cons (func (head args)) 
			(map (tail args) func)))

(def append (atom list)
	(if (tail list)
		(cons (head list) (append atom (tail list)))
		(cons (head list) atom)))

(def particle_emitter (args)
	(process_emitter (create_emitter) args))

(def process_emitter (emitter args)
	(process_emitter (process_emitter_property emitter (head args)) (tail args)))
