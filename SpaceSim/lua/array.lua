local array = {}

function array.add( arr, object )
	arr.count = arr.count + 1
	arr[arr.count] = object
end

function array.iterator( t )
	local i = 0
	local n = t.count
	return function ()
		i = i + 1
		if i <= n then return t[i] end
	end
end

function array.filter( arr, func )
	new_arr = {}
	local count = 0
	for element in array.iterator( arr ) do
		if func( element ) then
			count = count + 1
			new_arr[count] = element
		end
	end
	new_arr.count = count
	return new_arr
end

function array.tail( arr )
	local new_arr = {}
	for i,v in ipairs( arr ) do
		if i > 1 then
			table.insert( new_arr, v )
		end
	end
	new_arr.count = arr.count - 1
	return new_arr
end

function array.head( arr )
	return arr[1]
end

function array.new()
	local arr = { count = 0 }
	return arr
end

function array.foldr( arr, folder, final )
	if arr.count > 1 then
		return folder( array.head( arr ), array.foldr( array.tail( arr ), folder, final ))
	else
		return folder( array.head( arr ), final )
	end
end

function array.foldl( arr, folder, final )
	local arr_reversed = array.reverse( arr )
	array.foldr( arr_reversed, folder, final )
end

function array.reverse( arr )
	local arr_reversed = { count = arr.count }
	local i = 0
	while i < arr.count + 1 do
		arr_reversed[i] = arr[arr.count - i + 1]
		i = i + 1
	end
	return arr_reversed
end

function array.map( arr, func )
	local new_arr = array.new()
	array.foldl( arr, 
		function ( item, arr )
			array.add( arr, func( item ))
			return arr
		end,
		new_arr )
	return new_arr
end

function array.fold_map( folder, arr )
	local arr_new = array.new()
	array.foldr( function ( func, b )
		array.add( arr_new, func( folder ))
		end,
		arr )
	return arr_new
end

return array
