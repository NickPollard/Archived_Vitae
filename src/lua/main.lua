-- Test Lua script file

counter = 0

function handleKeyPress(keyCode)
	if keyCode == 97 then
		counter = counter - 1
	else
		counter = counter + 1
	end
	return counter
end
