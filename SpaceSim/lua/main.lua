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

-- Create a spacesim Game object
-- A gameobject has a visual representation (model), a physical entity for velocity and momentum (physic)
-- and a transform for locating it in space (transform)
function gameobject_create( model )
	vprint( "gameobject_create" )
	vprint( model )
	g = {}
	g.test = 256
	g.model = vcreateModelInstance( model )
	g.physic = vcreatePhysic()
	g.transform = vcreateTransform()
	g.body = vcreateBodySphere( g )
	vmodel_setTransform( g.model, g.transform )
	vphysic_setTransform( g.physic, g.transform )
	vbody_setTransform( g.body, g.transform )
	vscene_addModel( scene, g.model )
	vphysic_activate( engine, g.physic )
	v = Vector( 0.0, 0.0, 0.0, 0.0 )
	vphysic_setVelocity( g.physic, v )

	return g
end

function gameobject_destroy( g )
	vprint( "gameobject_destroy" )
	vprint( string.format( "Test num: %d", g.test ))
	vdeleteModelInstance( g.model )
	--vdestroyPhysic( g.physic )
	--vdestroyTransform( g.transform )
	--vdestroyBody( g.body )
end

function spawn_explosion( position )
	-- Attach a particle effect to the object
	local t = vcreateTransform()
	vexplosion( engine, t )
	-- Position it at the correct muzzle position and rotation
	vtransform_setWorldSpaceByTransform( t, position )
end

projectile_model = "dat/model/sphere.s"
function player_fire( p )
	local g = {}
	-- Create a new Projectile
	g = gameobject_create( projectile_model );

	-- Position it at the correct muzzle position and rotation
	vtransform_setWorldSpaceByTransform( g.transform, p.transform )

	-- Attach a particle effect to the object
	inTime( 0.2, function () vparticle_create( engine, g.transform ) end )

	-- Apply initial velocity
	bullet_speed = 150.0;
	ship_v = Vector( 0.0, 0.0, bullet_speed, 0.0 )
	world_v = vtransformVector( p.transform, ship_v )
	vphysic_setVelocity( g.physic, world_v );

	--inTime( 0.3, function () spawn_explosion( g.transform ) end );

	--[[
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
--]]
end

timers = {}
timers.count = 0

function addTimer( timer )
	timers.count = timers.count + 1
	timers[timers.count] = timer
end

---[[
function timer_create( time, action )
	timer = {
		time = time,
		action = action,
	}
	return timer
end
--]]

function inTime( time, action )
	addTimer( timer_create( time, action ))
end

function iterator( t )
	local i = 0
	local n = t.count
	return function ()
		i = i + 1
		if i <= n then return t[i] end
	end
end

function filter( array, func )
	new_array = {}
	local count = 0
	for element in iterator( array ) do
		if func( element ) then
			count = count + 1
			new_array[count] = element
		end
	end
	new_array.count = count
	return new_array
end

function timers_tick( dt )
	for element in iterator( timers ) do
		element.time = element.time - dt
		if element.time < 0 then
			element.action()
		end
	end

	new_timers = filter( timers, function( e ) return ( e.time > 0 ) end )
	timers = new_timers;
end

-- Create a player. The player is a specialised form of Gameobject
function playership_create()
	vprint( "playership_create" )
	local p = gameobject_create( "dat/model/ship_hd.s" )

	p.speed = 0.0
	return p
end

starting = true

-- Set up the Lua State
function init()
	vprint( "init" )
	starting = true
	library, msg = loadfile( "SpaceSim/lua/library.lua" )
	library()

--	vprint( tostring( msg ))
end

camera = "chase"
flycam = nil
chasecam = nil

function ship_tick( ship )
	vprint( "ship_tick" )
	ship_v = Vector( 0.0, 0.0, ship.speed, 0.0 )
	world_v = vtransformVector( ship.transform, ship_v )
	vphysic_setVelocity( ship.physic, world_v )
end

function vrand( lower, upper )
	return math.random() * ( upper - lower ) + lower
end

ships = {}
ship_count = 0

function ship_spawner()
	local ship = gameobject_create( "dat/model/ship_hd.s" )
	vbody_registerCollisionCallback( ship.body, ship_collisionHandler )
	ship.speed = 30.0
	vtransform_yaw( ship.transform, 3.14 )
	x = vrand( -50.0, 50.0 )
	y = vrand( 0.0, 100.0 )
	position = Vector( x, y, 100.0, 1.0 )
	vtransform_setWorldPosition( ship.transform, position )

	ships[ ship_count ] = ship
	ship_count = ship_count + 1

	inTime( 0.1, function () ship_tick( ship ) end )
	inTime( 3, ship_spawner )
end

function collision( this, other )
		
end

function ship_collisionHandler( ship, collider )
	spawn_explosion( ship.transform );
	ship_destroy( ship )

	--[[
	if collider == bullet then
		ship_destroy( ship )
	end
	--]]
end

function ship_destroy( ship )
	gameobject_destroy( ship )
	-- spawn explosion
end

function start()
	vprint( "start" )

	-- We create a player object which is a game-specific Lua lclass
	-- The player class itself creates several native C classes in the engine
	player_ship = playership_create()
	chasecam = vchasecam_follow( engine, player_ship.transform )
	flycam = vflycam( engine )
	vscene_setCamera( chasecam )

	--ship_spawner()
end

wave_interval_time = 10.0

function playership_tick()
	acceleration = 1.0
	yaw = 0.02
	pitch = 0.02
	width = 80
	if vkeyHeld( input, key.w )  or vtouchHeld( input, 000.0, -width*3, 100.0, -width*2 ) then
		player_ship.speed = player_ship.speed + acceleration
	end
	if vkeyHeld( input, key.s )  or vtouchHeld( input, 000.0, -width, 100.0, -1.0 ) then
		player_ship.speed = player_ship.speed - acceleration
	end
	if vkeyHeld( input, key.left ) or vtouchHeld( input, -width*3, -width*3, -width*2, -1.0 ) then
		vtransform_yaw( player_ship.transform, -yaw );
	end
	if vkeyHeld( input, key.right ) or vtouchHeld( input, -width, -width*3, -1.0, -1.0 ) then
		vtransform_yaw( player_ship.transform, yaw );
	end
	if vkeyHeld( input, key.up ) or vtouchHeld( input, -width*3, -width*3, -1.0, -width*2 ) then
		vtransform_pitch( player_ship.transform, -pitch );
	end
	if vkeyHeld( input, key.down ) or vtouchHeld( input, -width*3, -width, -1.0, -1.0 ) then
		vtransform_pitch( player_ship.transform, pitch );
	end

	-- Gunfire
	if vkeyPressed( input, key.space ) or vtouchPressed( input, 0.0, 0.0, 200.0, 200.0 ) then
		player_fire( player_ship )
	end


	ship_v = Vector( 0.0, 0.0, player_ship.speed, 0.0 )
	world_v = vtransformVector( player_ship.transform, ship_v )
	vphysic_setVelocity( player_ship.physic, world_v )
end

function toggle_camera()
	if camera == "chase" then
		vprint( "Activate Flycam" )
		camera = "fly"
		vscene_setCamera( flycam )
	else
		vprint( "Activate Chasecam" )
		camera = "chase"
		vscene_setCamera( chasecam )
	end
end

function debug_tick()
	if vkeyPressed( input, key.c ) then
		toggle_camera()
	end
--[[
	array_a = { 6, 5, 3, 4, 2, 1 }
	array_b = filter( array_a, function( e ) return (e % 2 == 0) end )
	vprint( "array_b:" )
	for key, value in ipairs(array_b) do
		vprint( tostring( value ))
	end
	--]]
end

-- Called once per frame to update the current Lua State
function tick()
	if starting then
		starting = false
		start()
	end

	playership_tick()

	debug_tick()

	dt = 0.033
	timers_tick( dt )
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
