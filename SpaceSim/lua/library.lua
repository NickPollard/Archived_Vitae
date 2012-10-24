local library = {}

function library.vrand( lower, upper )
	return math.random() * ( upper - lower ) + lower
end

return library
