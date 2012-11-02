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
	new_arr = {}
	for i,v in ipairs( arr ) do
		if i > 1 then
			table.insert( new_arr, v )
		end
	end
	new_arr.n = arr.n - 1
	return new_arr
end

function array.new()
	local arr = { count = 0 }
	return arr
end

function array.fold( folder, arr )
	if arr.count > 0 then
		folder( array.head( arr ), array.fold( folder, array.tail( arr )))
	end
end

function array.map( func, arr )
	local new_arr = array.new()
	array.fold( function ( item )
		array.add( new_arr, func( item ))
	end )
	return new_arr
end

function array.fold_map( folder, arr )
	local arr_new = array.new()
	array.fold( function ( func, b )
		array.add( arr_new, func( folder ))
		end,
		arr )
	return arr_new
end

return array
