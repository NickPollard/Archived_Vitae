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
	(map args (lambda (arg) (process_property arg emitter))))

----

(particle_emitter '((property "size" '((0.0 1.0)
										(1.0 5.0)))))

(def particle_emitter (properties)
	(process_emitter (create_emitter) properties))

(process_emitter (emitter properties)
	(map properties (lambda (arg) (process_property arg emitter))))

(property "size" '('(0.0 '(1.0))
						'(1.0 '(5.0))
						'(2.0 '(10.0))))


(def property (name values)
	((lambda (property)
		(map values (lambda (value)
						(property_add property value))))
	(create_property (length (head (tail (head values)))))))

	(data (int num_values)
			(array int num_values values))
	
(model '((mesh "filename.obj")))

; comment

; array - takes a list of stuff and puts it as an array

; blob - takes a list of stuff and puts it as a contigious blob

(def total-size (elements)
	(reduce sum (map contents (data-size)))

; this one probably has to go to C
; copies the data from content to blob
; increments the blob write head
(def blob-add (blob content)
	)

(def blob-fill (blob contents)
	(if (empty contents)
		blob
		(blob-fill (blob-add blob (head contents))
					(tail contents))))

(def blob (contents)
	(blob-fill (create-blob (total-size contents)) contents))

(def model (meshes)
	(blob (map meshes load_mesh)))

;
;
;
;
;
;
;
;
;
;
; What do I actually want to do with the lisp loading system?
;
; I take inputs in the form of an object-based system, where I have named properties who's
; values are programmatically created through lisp functions
; e.g.
;
; (object (size value)
;			(color value)
;			(lifetime value))
;
; I want to convert these (eventually) into a compact C struct of data
; e.g.
;
;	struct object {
;		float size;
;		vector color;
;		float lifetime;
;		};
;
; I want to be able to support having data out-of-order in the lisp code
; I want to be able to support missing data (ie. use default initialization)
; I want to automate as much as possible, so that I don't have to write lots of
; serialization functions for each type (though I could metaprogram some)
;
; One thing I can do is produce a generic property, which contains a name, type, and
; value. Eg. 'size', float, 0.5f
; So the first step could be to convert the initial lisp tree into an intermediate
; list of properties
; e.g.
; (property "object" struct ((property "size" float 0.5f)
;							(property "color" vector (vector 0.f 0.f 1.f))
;							(property "lifetime" float 1.f)))
;
; This lisp code describes what the C data will contain
; To convert this intermediate data languate into a C blob we then need to do some more
; work
; How can we do this?
; We need to map object names to offsets in the struct
; I've already looked at this, have a preprocessing step build a map of (object, property)
; pairs to offsets
; Then it's just a case of mapping all of them to memory
; The (property ...) list function would be implemented in C, everything else could be done
; in lisp
;
;	(object "type" ((property "size" float 0.5)
;					(property "color" vector (vector 0.0 0.0 1.0))
;					(property "lifetime" float 1.0)))
;

void l_object( term* type, term* property_list ) {
	// Preconditions
	assert( type && type->head );

	const char* typename = type->head;
	// create_type just uses a map of typename strings to functions (that create the structs)
	void* struct_object = create_type( typename );

	// call write_property for every property in the list
	term* p = property_list;
	while ( p ) {
			// the property_x functions just get the value by the correct number of head() and tail() calls
			write_property( struct_object, property_name( p ), property_type( p ), property_value( p ));
		}
	}

void write_property( void* object, const char* name, const char* type, void* value ) {
	size_t offset = property_offset( object_name, name );	
	memcpy( object + offset, value, sizeof(type))
	}

;
; OK, that looks pretty sweet. But how do I get from the initial Lisp layout to the intermediate
; object code?
;
; To remind myself, I start with:
;
; (particle ((size 0.5)
;			(color (vector 0.0 0.0 1.0))
;			(lifetime value)))
;
; And want to get to:
;
;	(object "type" ((property "size" float 0.5)
;					(property "color" vector (vector 0.0 0.0 1.0))
;					(property "lifetime" float 1.0)))
;

(def particle (properties)
	(map properties make_property))

; input is a list of the form:
; (atom value)
(def make_property (input)
	'(property (head input) (value_type (tail input)) (tail input)))
	
