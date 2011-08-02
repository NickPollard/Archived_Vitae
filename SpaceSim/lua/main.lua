-- SpaceSim main game lua script

--[[

This file contains the main game routines for the SpaceSim game.
Most game logic is written in Lua, whilst the core numerical processing (rendering, physics, animation etc.)
are handled by the Vitae engine in C.

Lua should be able to do everything C can, but where performance is necessary, code should be rewritten in
C and only controlled remotely by Lua

]]--

-- player - this object contains general data about the player
player = nil
-- player_ship - the actual ship entity flying around
player_ship = nil

--[[ Vitae C functions exposed for Lua 

vitae_model_load( model )		-		Takes a string containing the model filename, creates a modelinstance of that model and registers it to be updated
vitae_transform( )							- Creates a new transform and registers it for updating
vitae_physic( )								- Creates a new physic and registers it for updating
vitae_model_setTransform( model, transform )		- Sets the transform of the modelinstance <model> to be <transform>

]]--

-- Create a spacesim Game object
-- A gameobject has a visual representation (model), a physical entity for velocity and momentum (physic)
-- and a transform for locating it in space (transform)
function gameobject_create( model )
	vprint( "gameobject_create" )
	vprint( model )
	local g = {}
	g.model = vcreateModelInstance( model )
	g.physic = vcreatePhysic()
	g.transform = vcreateTransform()
	vmodel_setTransform( g.model, g.transform )
	vphysic_setTransform( g.physic, g.transform )
	vscene_addModel( scene, g.model )
	vphysic_activate( g.physic )
	vphysic_setVelocity( g.physic, 0.0, 0.0, 0.0 )
	return g
end

function player_accelerate( p, a )
	vitae_physic_accelerate( p.physic, a )
end

function player_yaw( p, y )
	vitae_physic_yaw( p.physic, y )
end

projectile_model = "dat/model/smoothsphere2.obj"
--[[
function player_fire( p )
	local g = {}
	-- Create a new Projectile
	g = gameobject_create( projectile_model );
	-- Position it at the correct muzzle position and rotation
	gun_transform = vitae_attach_transform( g.model, "bullet_spawn" )
	vitae_transform_setWorldSpace( g.transform, gun_transform )
	speed = 10.0
	velocity = gun_transform * vector( 0.0, 0.0, speed, 0.0 )
	vitae_physic_setVelocity( g.physic, vitae_attach_position( g.model, "bullet_spawn" ) )
	
	vitae_physic_onCollision( g.physic, function ( bullet, target )
		gameobject_destroy( bullet )
		gameobject_destroy( target )
		vitae_particle_spawn( "explosion.part", translation( bullet.transform ) )
	end )
end
--]]

-- Create a player. The player is a specialised form of Gameobject
function playership_create()
	vprint( "playership_create" )
	local p = gameobject_create( "dat/model/ship.obj" )
	--[[
	vitae_register_keybind( "accelerate", "w", player_accelerate( p, acceleration ) )
	vitae_register_keybind( "decelerate", "s", player_accelerate( p, -acceleration ) )
	vitae_register_keybind( "yaw left", "a", player_yaw( p, yaw) )
	vitae_register_keybind( "yaw right", "d", player_yaw( p, -yaw) )
	vitae_register_keybind( "fire", "space", player_fire( p ) )
--]]
	p.speed = 0.0
	return p
end

starting = true

-- Set up the Lua State
function init()
	vprint( "init" )
	starting = true
end

function start()
	vprint( "start" )

	-- We create a player object which is a game-specific Lua class
	-- The player class itself creates several native C classes in the engine
	player_ship = playership_create()
end

wave_interval_time = 10.0

function playership_tick()
	acceleration = 1.0
	yaw = 0.01
	if vkeyHeld( key.up ) then
		player_ship.speed = player_ship.speed + acceleration
	end
	if vkeyHeld( key.down ) then
		player_ship.speed = player_ship.speed - acceleration
	end
	if vkeyHeld( key.left ) then
		vtransform_yaw( player_ship.transform, -yaw );
	end
	if vkeyHeld( key.right ) then
		vtransform_yaw( player_ship.transform, yaw );
	end
--	vphysic_setVelocity( player_ship.physic, 0.0, 0.0, player_ship.speed )
---[[
	ship_v = Vector( 0.0, 0.0, player_ship.speed, 0.0 )
	world_v = vtransformVector( player_ship.transform, ship_v )
	vphysic_setVelocity( player_ship.physic, world_v )
	--]]
end

-- Called once per frame to update the current Lua State
function tick()
	if starting then
		vprint( "tick" )
		starting = false
		start()
	end

	playership_tick()
--[[
	if wave_complete( current_wave ) then
		current_wave = current_wave + 1
		vitae_countdown_trigger( wave_interval_time, spawn_wave( current_wave ))
	end
--]]
end

-- Called on termination to clean up after itself
function terminate()
	player = nil
end

-- We want to have set waves
-- Waves spawn only when the previous wave is complete
-- In a wave, not everything spawns immediately
-- Want to be able to set up spawners and tag them with waves to be active on

--[[
function spawner( spawn_type, amount, time )
	local spawner = {
		spawn_type = spawn_type,
		amount = amount,
		time = time
	}
	return spawner
end
--]]

function spawn( spawner )
	for i=1,spawner.amount do
		gameobject( spawner.spawn_type )
	end
end

function wave_add_spawn( wave, spawner )
	wave[wave.count] = spawner
	wave.count = wave.count + 1
end

function wave()
	wave = {}
	return wave
end

function setup_wave_spawns()
	--[[
	wave_1 = wave()
	wave_add_spawn( wave_1, spawner( "rocket_fighter", 5, 10.0 ) )
	--]]

	wave = { count = 0 }
	wave[wave.count] = spawner( "rocket_fighter", 5, 10.0 )
	wave[wave.count] = function ( count, repeat_time )
		spawn_gameobject( "rocket_fighter", repeat_time )
		if count > 1 then
			delayed_trigger( wave[wave.count]( count-1, repeat_time ) )
		end
	end
	wave.count = wave.count + 1

	function start_spawn_countdown( spawner )
		vitae_countdown_trigger( spawner.time, spawn( spawner ))
	end
end

function start_wave( wave )
	for i=1,wave.count do
		start_spawn_countdown( wave[i] )
	end
end

function delay( time, command )
	if time <= 0 then
		command()
	else
		print( string.format( "Delay timer: %d", time ))
		delay( time-1, command )
	end
end

function test( )
	--[[
	func = function( a, b )
		if a > 0 then
			print( b )
			func( a - 1, b )
		end	
	end
	func( z, y )

	--]]

	wave = { count = 0 }
	wave[wave.count] = function () 
		spawner = function ( spawn_type, count, repeat_time )
			print( string.format( "Spawning %s", spawn_type ) )
			if count > 1 then
				delay( repeat_time, function ()
					spawner( spawn_type, count-1, repeat_time ) 
				end )
			end
		end 
		spawner( "rocket", 5, 10 )
	end

	wave[0]()
end
