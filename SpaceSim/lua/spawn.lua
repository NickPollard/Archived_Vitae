local spawn = {}

spawn.random = 0 

function spawn.init()
	spawn.random = vrand_newSeq()
end

function spawn.spawnSpacePositions( spawn_space )
	local positions = array.new()
	local x = -spawn_space.width
	while x <= spawn_space.width do
		local y = 0
		while y < spawn_space.height do
			local position = { x = x, y = y }
			array.add( positions, position )
			y = y + 1
		end
		x = x + 1
	end
	return positions
end

---[[
function spawn.positionerTurret( spawn_space, current_positions )
	-- Pick the most central floor space
	local positions = spawn.spawnSpacePositions( spawn_space )
	vprint( "positioner turret" )
	local available_positions = array.filter( positions,
		function ( position )
			return not array.contains( current_positions, 
				function ( item )
					vprint( "func!") 
					vprint( "position " .. position.x .. ", " .. position.y )
					vprint( "item " .. item.x .. ", " .. item.y )
					return item.x == position.x and item.y == position.y 
				end )
		end )
	vprint( "positioner turret 2" )
	local ranked_positions = array.rank( positions, 
		function( position )
			return - math.abs( position.x ) - ( position.y ) * spawn_space.width
		end )
	vprint( "positioner turret using position " .. ranked_positions[1].x .. ", " .. ranked_positions[1].y )
	array.add( current_positions, ranked_positions[1] )
	return current_positions
end
--]]

function spawn.positionerDefault( spawn_space, current_positions )
	local u_delta = 20.0
	local new_position = { u = current_positions[current_positions.count].u + u_delta, v = spawn_space.v }
	array.add( current_positions, new_position )
	return current_positions
end

function spawn.positionsForGroup( v, spawn_space, spawn_group_positioners )
	local current_positions = array.new()
	local spawn_positions = array.foldr( spawn_group_positioners,
					function ( positioner, positions )
						local p = positioner( spawn_space, positions )
						return p
					end,
					current_positions )

	vprint( "calculated positions." )

	return spawn_positions
end

function spawn.spawnGroup( spawn_group, v )
	local spawn_space = { v = v, width = 3, height = 3, u_delta = 20.0, v_delta = 20.0 }
	local spawn_positions = spawn.positionsForGroup( v, spawn_space, spawn_group.positioners )

	for i, e in ipairs( spawn_positions ) do
		vprint( "[" .. i .. "] ( " .. e.x .. ", " .. e.y .. " )" )
	end

	local world_positions = array.map( spawn_positions, function( spawn_pos )
			local position = { u = spawn_pos.x * spawn_space.u_delta, v = v, y = spawn_pos.y }
			return position
		end )
	
	for i, e in ipairs( world_positions ) do
		vprint( "[" .. i .. "] ( " .. e.u .. ", " .. e.v .. " )" )
	end

	library.apply_list( spawn_group.spawners, world_positions )
	vprint( "spawned units." )
end

function spawn.spawnTurret( u, v )
	vprint( "u " .. u )
	vprint( "v " .. v )
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
	--[[
	if r > 0.75 then
		return function( coord ) spawn_interceptor( coord.u, coord.v, interceptor_attack_homing ) end
	elseif r > 0.4 then
		return function( coord ) spawn_interceptor( coord.u, coord.v, interceptor_attack_gun ) end
	else
	--]]
		return function( coord ) spawn.spawnTurret( coord.u, coord.v ) end
	--end
end

function spawn.generateSpawnGroupForDifficulty( difficulty )
	-- For now, assume difficulty is number of units to spawn
	local group = {}
	group.spawners = { count = difficulty }
	group.positioners = { count = difficulty }
	local i = 1
	while i <= difficulty do
		group.spawners[i] = spawn.randomEnemy()
		group.positioners[i] = spawn.positionerTurret
		i = i + 1
	end
	return group
end

return spawn
