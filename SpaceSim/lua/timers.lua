local timers = { timers = { count = 0} }

function timers.add( timer )
	timers.timers.count = timers.timers.count + 1
	timers.timers[timers.timers.count] = timer
end

function timers.create( time, action )
	timer = {
		time = time,
		action = action,
	}
	return timer
end

function timers.tick( dt )
	for element in array.iterator( timers.timers ) do
		element.time = element.time - dt
		if element.time < 0 then
			element.action()
		end
	end

	new_timers = array.filter( timers.timers, function( e ) return ( e.time > 0 ) end )
	timers.timers = new_timers;
end

return timers
