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

library.rolling_average = {}

function library.rolling_average.create( max )
	local rolling = { max = max, arr = array.new() }
	return rolling
end

function library.rolling_average.add( rolling, value )
	array.add( rolling.arr, value )
	while rolling.arr.count > rolling.max do
		rolling.arr = array.tail( rolling.arr )
	end
end

function library.rolling_average.sample( rolling )
	if rolling.arr.count > 0 then
		local sample = array.sum( rolling.arr )
		return sample / rolling.arr.count
	else
		return nil
	end
end

function library.rolling_average.last( rolling )
	if rolling.arr.count > 0 then
		vprint( "rolling_average.last " .. rolling.arr[rolling.arr.count] )
		return rolling.arr[rolling.arr.count]
	else
		return nil
	end
end

function library.modf( value, mod ) 
	local m = math.floor( value / mod )
	return value - m * mod
end

function library.roundf( value, mod ) 
	return value - library.modf( value, mod )
end

return library
