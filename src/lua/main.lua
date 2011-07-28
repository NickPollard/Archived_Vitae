-- Test Lua script file

counter = 0

function init()
	counter = 42

--	registerEventHandler(engine, onTick, "tick")
	registerEventHandler()

	spawn = true
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

function start()
	t = vcreateModelInstance( "dat/model/smoothsphere2.obj" )
	vsetWorldSpacePosition( t, 0.0, 10.0, 0.0 )
	spawn = false

	vprint( string.format("%d %d %d %d", key.w, key.a, key.s, key.d) )
end

function tick()
	counter = counter + 1
	if spawn then
		start()
	end

	if vkeyPressed( key.a ) then
		onPressA()
	end
end

function onPressA()
	vprint( "A pressed!" )
end
