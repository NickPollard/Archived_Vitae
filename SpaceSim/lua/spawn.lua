local spawn = {}

spawn.group_two_interceptors = { count = 2 }
spawn.group_two_interceptors[1] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end
spawn.group_two_interceptors[2] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end

spawn.group_two_turrets = { count = 2 }
spawn.group_two_turrets[1] = function( u, v ) spawn_turret( u, v ) end
spawn.group_two_turrets[2] = function( u, v ) spawn_turret( u, v ) end

function spawn.spawnGroup( spawn_group, v )
	local u_delta = 20.0
	local u = 0.0
	for spawner in iterator( spawn_group ) do
		spawner( u, v )
		u = u + u_delta
	end
end

return spawn
