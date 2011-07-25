-- Test Lua script file

counter = 0

function init()
	counter = 42

--	registerEventHandler(engine, onTick, "tick")
	registerEventHandler()

--	vprint( "Hello" )
end

function handleKeyPress(keyCode)
	if keyCode == 97 then
		counter = counter - 1
	else
		counter = counter + 1
	end
	return counter
end

function tick()
	counter = counter + 1
--	print( "tick" )
end
