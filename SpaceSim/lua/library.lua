local library = {}

function library.vrand( lower, upper )
	return math.random() * ( upper - lower ) + lower
end

function library.apply_list( functions, arguments )
	local total = functions.count
	local i = 1
	while i <= total do
		functions[i]( arguments[i] )
		i = i + 1
	end
end

function library.contains( value, range_a, range_b )
	range_max = math.max( range_a, range_b )
	range_min = math.min( range_a, range_b )
	return ( value < range_max ) and ( value >= range_min )
end

return library
