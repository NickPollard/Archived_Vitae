local spawn = {}

spawn.group_two_interceptors = { count = 2 }
spawn.group_two_interceptors[1] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end
spawn.group_two_interceptors[2] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end

spawn.group_two_turrets = { count = 2 }
spawn.group_two_turrets[1] = function( u, v ) spawn.spawnTurret( u, v ) end
spawn.group_two_turrets[2] = function( u, v ) spawn.spawnTurret( u, v ) end

spawn.group_turrets_and_interceptors = { count = 4 }
spawn.group_turrets_and_interceptors[1] = function( u, v ) spawn.spawnTurret( u, v ) end
spawn.group_turrets_and_interceptors[2] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end
spawn.group_turrets_and_interceptors[3] = function( u, v ) spawn.spawnTurret( u, v ) end
spawn.group_turrets_and_interceptors[4] = function( u, v ) spawn_interceptor( u, v, interceptor_attack_gun ) end

function spawn.spawnGroup( spawn_group, v )
	local u_delta = 20.0
	local u = 0.0
	for spawner in iterator( spawn_group ) do
		spawner( u, v )
		u = u + u_delta
	end
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
	if i < 3 then
		return spawn.group_two_turrets
	end
	if i < 5 then
		return spawn.group_two_interceptors
	end
	return spawn.group_turrets_and_interceptors
end

return spawn
