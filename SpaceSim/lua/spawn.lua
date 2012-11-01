local spawn = {}

spawn.group_two_interceptors = {}
spawn.group_two_interceptors.spawners = { count = 2 }
spawn.group_two_interceptors.spawners[1] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end
spawn.group_two_interceptors.spawners[2] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end

spawn.group_two_turrets = {}
spawn.group_two_turrets.spawners = { count = 2 }
spawn.group_two_turrets.spawners[1] = function( u, v ) spawn.spawnTurret( u, v ) end
spawn.group_two_turrets.spawners[2] = function( u, v ) spawn.spawnTurret( u, v ) end

spawn.group_turrets_and_interceptors = {}
spawn.group_turrets_and_interceptors.spawners = { count = 4 }
spawn.group_turrets_and_interceptors.spawners[1] = function( u, v ) spawn.spawnTurret( u, v ) end
spawn.group_turrets_and_interceptors.spawners[2] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end
spawn.group_turrets_and_interceptors.spawners[3] = function( u, v ) spawn.spawnTurret( u, v ) end
spawn.group_turrets_and_interceptors.spawners[4] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end

spawn.random = 0 

function spawn.init()
	spawn.random = vrand_newSeq()
end

function spawn.positionsForGroup( v, spawn_group_positioners )
	--[[
	local spawn_space = { width = 9, height = 3, u_delta = 20.0, v_delta = 20.0 }
	local spawn_positions = fold_map( spawn_space, spawn_group_positioners )
	--]]

	local spawn_positions = { count = spawn_group_positioners.count }
	local i = 1
	local u = 0.0
	local u_delta = 20.0
	while i <= spawn_positions.count do
		spawn_positions[i] = { u = u, v = v }
		u = u + u_delta
		i = i + 1
	end
	return spawn_positions
end

function spawn.spawnGroup( spawn_group, v )
---[[
	vprint( "spawning group" )
	spawn_positions = spawn.positionsForGroup( v, spawn_group.positioners )
	vprint( "spawning group 1" )
	library.apply_list( spawn_group.spawners, spawn_positions )
	vprint( "spawning group 2" )
	--]]

--[[
	local u_delta = 20.0
	local u = 0.0
	local i = 0.5
	local side = 1.0
	for spawner in iterator( spawn_group.spawners ) do
		spawner( u + side * math.floor( i ) * u_delta, v )
		i = i + 0.5
		side = side * -1.0
	end
	--]]
end

function spawn.spawnTurret( u, v )
	local spawn_height = 0.0

	-- position
	local x, y, z = vcanyon_position( u, v )
	local position = Vector( x, y + spawn_height, z, 1.0 )
	local turret = gameobject_create( "dat/model/gun_turret.s" )
	vtransform_setWorldPosition( turret.transform, position )

	-- Orientation
	local facing_x, facing_y, facing_z = vcanyon_position( u, v - 1.0 )
	local facing_position = Vector( facing_x, y + spawn_height, facing_z, 1.0 )
	vtransform_facingWorld( turret.transform, facing_position )

	-- Physics
	vbody_registerCollisionCallback( turret.body, turret_collisionHandler )
	vbody_setLayers( turret.body, collision_layer_enemy )
	vbody_setCollidableLayers( turret.body, collision_layer_player )

	turret.tick = turret_tick
	turret.cooldown = turret_cooldown

	-- ai
	turret.behaviour = turret_state_inactive

	turrets.count = turrets.count + 1
	turrets[turrets.count] = turret
end

function spawn.spawnGroupForIndex( i )
	return spawn.generateSpawnGroupForDifficulty( i )
end


function spawn.randomEnemy()
	local r = vrand( spawn.random, 0.0, 1.0 )
	if r > 0.75 then
		return function( coord ) spawn_interceptor( coord.u, coord.v, interceptor_attack_homing ) end
	elseif r > 0.4 then
		return function( coord ) spawn_interceptor( coord.u, coord.v, interceptor_attack_gun ) end
	else
		return function( coord ) spawn.spawnTurret( coord.u, coord.v ) end
	end
end

function spawn.generateSpawnGroupForDifficulty( difficulty )
	-- For now, assume difficulty is number of units to spawn
	local group = {}
	group.spawners = { count = difficulty }
	group.positioners = { count = difficulty }
	local i = 1
	while i <= difficulty do
		group.spawners[i] = spawn.randomEnemy()
		i = i + 1
	end
	return group
end

return spawn
