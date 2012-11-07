local triggers = { 
	triggers = { count = 0 }
}
function triggers.add( trigger )
	array.add( triggers.triggers, trigger )
end

function triggers.create( trigger, action )
	local trigger = {
		trigger = trigger,
		action = action,
		triggered = false
	}
	return trigger
end

function triggers.tick( dt )
	for trigger in array.iterator( triggers.triggers ) do
		if trigger.trigger() then
			vprint( "Trigger!" )
			trigger.action()
			trigger.triggered = true
		end
	end
	triggers.triggers = array.filter( triggers.triggers, 
		function( trigger )
			return trigger.triggered ~= true
		end
	)
end

return triggers
