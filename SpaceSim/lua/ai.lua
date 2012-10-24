local ai = {}

function ai.combinator( f )
	return f( f )
end
ai.count = 1

function ai.test( f )
	ai.count = ai.count + 1
	vprint( "count " .. ai.count )
	if ai.count > 10 then
		vprint( "##1" )
		return function ( x ) vprint( "End" ) return nil end
	else
		vprint( "##2" )
		return f
	end
end

function ai.test_combinator()
	ai.count = 1
	func = ai.test
	vprint( "//////////////////////////////////////////// AI." )
	while true do
		func = ai.combinator( func )
	end
end

--------------------------------------------------

function ai.dodge( )
	vprint( "dodge" )
end

function ai.shoot( )
	vprint( "shoot" )
end

function ai.state( tick, transition )
	return function ( entity, dt )
		tick( entity, dt )
		return transition()
	end
end

function ai.queue3( a, b, c )
	state_a = ai.state( a, function() return state_b end )
	state_b = ai.state( b, function() return state_c end )
	state_c = ai.state( c, function() return nil end )
	return state_a
end

function ai.queue( ... )
	return ai.queue_internal( arg )
end

function tail( array )
	new_array = {}
	for i,v in ipairs( array ) do
		if i > 1 then
			table.insert( new_array, v )
		end
	end
	new_array.n = array.n - 1
	return new_array
end

function ai.queue_internal( args )
	vprint( "queue_internal" )
	local state = nil
	local next_state = nil
	if args.n > 0 then
		next_state = ai.queue_internal( tail( args )) 
		state = ai.state( args[1], function() return next_state end )
	else
		state = nil
	end
	return state
end

function ai.test_states()
	vprint( "### ai test states" )

	--[[
	func = ai.state_combinator( ai.dodge, ai.shoot )
	while true do
		func = func( func )
	end
	--]]

	--[[
	test_state = ai.queue3( function() vprint( "MoveTo" ) end, 
						function() vprint( "Attack" ) end, 
						function() vprint( "Exit" ) end )
						--]]

	test_state = ai.queue( function() vprint( "MoveTo" ) end, 
						function() vprint( "Attack" ) end, 
						function() vprint( "Defend" ) end, 
						function() vprint( "Exit" ) end )

	vprint( "queue_internal test" )
	i = 0
	while i < 10 do
		test_state = test_state()
		i = i + 1
	end
end

ai.dead = nil
ai.dead = ai.state( function () end, function () return ai.dead end )

return ai
