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


return library
