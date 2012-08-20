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
function gameobject_create( model_file )
	g = {}
	g.test = 256
	g.model = vcreateModelInstance( model_file )
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
	inTime( 0.3, function () vdeleteModelInstance( g.model ) 
							vdestroyTransform( g.transform )
							--vdestroyPhysic( g.physic )
				end )
	vdestroyBody( g.body )
end

function spawn_explosion( position )
	-- Attach a particle effect to the object
	local t = vcreateTransform()
	vparticle_create( engine, t, "dat/script/lisp/explosion.s" )
	vparticle_create( engine, t, "dat/script/lisp/explosion_b.s" )
	vparticle_create( engine, t, "dat/script/lisp/explosion_c.s" )
	-- Position it at the correct muzzle position and rotation
	vtransform_setWorldSpaceByTransform( t, position )
end

projectile_model = "dat/model/missile.s"
weapons_cooldown = 0.5

function player_fire( ship )
	if ship.cooldown <= 0.0 then
		muzzle_pos = Vector( 3.0, 0.0, 0.0, 1.0 );
		fire_missile( ship, muzzle_pos );
		muzzle_pos = Vector( -3.0, 0.0, 0.0, 1.0 );
		fire_missile( ship, muzzle_pos );
		ship.cooldown = weapons_cooldown
	end
end

function fire_missile( ship, offset )
	local projectile = {}
	-- Create a new Projectile
	projectile = gameobject_create( projectile_model );
	vbody_setLayers( projectile.body, collision_layer_player )
	vbody_setCollidableLayers( projectile.body, collision_layer_enemy )

	-- Position it at the correct muzzle position and rotation
	muzzle_world_pos = vtransformVector( ship.transform, muzzle_pos )
	vtransform_setWorldSpaceByTransform( projectile.transform, ship.transform )
	vtransform_setWorldPosition( projectile.transform, muzzle_world_pos )

	-- Attach a particle effect to the object
	vparticle_create( engine, projectile.transform, "dat/script/lisp/missile_glow.s" )
	inTime( 0.2, function () vparticle_create( engine, projectile.transform, "dat/script/lisp/missile_particle.s" ) end )

	-- Apply initial velocity
	bullet_speed = 150.0;
	ship_v = Vector( 0.0, 0.0, bullet_speed, 0.0 )
	world_v = vtransformVector( ship.transform, ship_v )
	vphysic_setVelocity( projectile.physic, world_v );
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
	local p = gameobject_create( "dat/model/ship_hd.s" )
	p.speed = 0.0
	p.cooldown = 0.0
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

function setup_controls()
	-- Set up steering input for the player ship
	use_drag = false
	if touch_enabled then
		if use_drag then
			-- Steering
			local w = 720
			local h = 720
			local x = 1280 - w
			local y = 720 - h
			player_ship.joypad_mapper = drag_map()
			player_ship.joypad = vcreateTouchPad( input, x, y, w, h )
			player_ship.steering_input = steering_input_drag
			-- UI drawing is upside down compared to touchpad placement - what to do about this?
			vcreateUIPanel( engine, x, 0, w, h )

		else
			-- Steering
			local w = 360
			local h = 360
			local x = 1280 - w
			local y = 720 - h
			local deadzone = 30
			player_ship.joypad_mapper = joypad_mapSquare( w, h, deadzone, deadzone )
			player_ship.joypad = vcreateTouchPad( input, x, y, w, h )
			player_ship.steering_input = steering_input_joypad
			-- UI drawing is upside down compared to touchpad placement - what to do about this?
			vcreateUIPanel( engine, x, 0, w, h )

		end			-- Firing Trigger
		x = 0
		y = 30
		w = 200
		h = 200
		player_ship.fire_trigger = vcreateTouchPad( input, x, y, w, h )
		vcreateUIPanel( engine, x, 720-y-h, w, h )
		-- throttle	
		vcreateUIPanel( engine, 0, 200, 100, 100 )
		vcreateUIPanel( engine, 0, 0, 100, 100 )
	else
		player_ship.steering_input = steering_input_keyboard
	end
end

function player_ship_collisionHandler( ship, collider )
	-- stop the ship
	ship.speed = 0.0
	local no_velocity = Vector( 0.0, 0.0, 0.0, 0.0 )
	vphysic_setVelocity( ship.physic, no_velocity )

	-- destroy it
	spawn_explosion( ship.transform )
	gameobject_destroy( ship )

	-- queue a restart
	inTime( 2.0, restart )
end

collision_layer_player = 1
collision_layer_enemy = 2

function restart()
	-- We create a player object which is a game-specific Lua class
	-- The player class itself creates several native C classes in the engine
	player_ship = playership_create()
	local no_velocity = Vector( 0.0, 0.0, 0.0, 0.0 )
	vphysic_setVelocity( player_ship.physic, no_velocity )

	vbody_registerCollisionCallback( player_ship.body, player_ship_collisionHandler )
	vbody_setLayers( player_ship.body, collision_layer_player )
	vbody_setCollidableLayers( player_ship.body, collision_layer_enemy )

	setup_controls()
	vtransform_yaw( player_ship.transform, math.pi * 2 * 1.32 );
	chasecam = vchasecam_follow( engine, player_ship.transform )
	flycam = vflycam( engine )
	vscene_setCamera( chasecam )
end

function start()
	restart()

	-- Test spawns
	spawn_v = -3.0
	local width = 25.0
	while spawn_v < 3.0 do
		spawn_v = spawn_v + 0.3
		--spawn_target( spawn_v )
		spawn_atCanyon( -width, spawn_v, "dat/model/skyscraper.s" )
		spawn_atCanyon( width, spawn_v, "dat/model/skyscraper.s" )
	end
	--entities_spawnAll( -750.0, 750.0 )

	--ship_spawner()
end

wave_interval_time = 10.0

function sign( x )
	if x > 0 then
		return 1.0
	else
		return -1.0
	end
end

-- maps a touch input on the joypad into a joypad tilt
-- can have a defined deadzone in the middle (resulting in 0)
function joypad_mapSquare( width, height, deadzone_x, deadzone_y )
	return function( x, y ) 	
		center_x = width / 2
		center_y = height / 2
		x = sign( x - center_x ) * math.max( math.abs( x - center_x ) - deadzone_x, 0.0 ) / (( width - deadzone_x ) / 2 )
		y = sign( y - center_y ) * math.max( math.abs( y - center_y ) - deadzone_y, 0.0 ) / (( height - deadzone_y ) / 2 )
		return x,y
	end
end

function steering_input_joypad()
	-- Using Joypad
	local yaw = 0.0
	local pitch = 0.0
	touched, joypad_x, joypad_y = vtouchPadTouched( player_ship.joypad )
	if touched then
		yaw, pitch = player_ship.joypad_mapper( joypad_x, joypad_y )
		vprint( "inputs mapped " .. yaw .. " " .. pitch )
	end
	return yaw, pitch
end


function steering_input_drag()
	-- Using Joypad
	local yaw = 0.0
	local pitch = 0.0
	dragged, drag_x, drag_y = vtouchPadDragged( player_ship.joypad )
	if dragged then
		yaw, pitch = player_ship.joypad_mapper( drag_x, drag_y )
		vprint( "inputs mapped " .. yaw .. " " .. pitch )
	end
	return yaw, pitch
end

function drag_map()
	return function( x, y )
		x_scale = 15.0
		y_scale = 15.0
		return x / x_scale, y / y_scale
	end
end


function steering_input_keyboard()
	local yaw = 0.0
	local pitch = 0.0
	-- Steering
	if vkeyHeld( input, key.left ) then
		yaw = -1.0
	end
	if vkeyHeld( input, key.right ) then
		yaw = 1.0
	end
	if vkeyHeld( input, key.up ) then
		pitch = -1.0
	end
	if vkeyHeld( input, key.down ) then
		pitch = 1.0
	end
	return yaw, pitch
end

function playership_tick( ship, dt )
	acceleration = 16.0
	yaw_per_second = 1.5 
	pitch_per_second = 1.5
	width = 100

	local input_yaw = 0.0
	local input_pitch = 0.0
	input_yaw, input_pitch = ship.steering_input()

	-- set to -1.0 to invert
	invert_pitch = 1.0
	pitch = invert_pitch * input_pitch * pitch_per_second * dt;
	yaw = input_yaw * yaw_per_second * dt;
	delta_speed = acceleration * dt;

	-- throttle
	if vkeyHeld( input, key.w )  or vtouchHeld( input, 000.0, -width*3, 100.0, -width*2 ) then
		ship.speed = ship.speed + delta_speed
	end
	if vkeyHeld( input, key.s )  or vtouchHeld( input, 000.0, -width, 100.0, -1.0 ) then
		ship.speed = ship.speed - delta_speed
	end

	vtransform_yaw( ship.transform, yaw );
	vtransform_pitch( ship.transform, pitch );

	-- Gunfire
	local fired = false
	if touch_enabled then
		fired, joypad_x, joypad_y = vtouchPadTouched( ship.fire_trigger )
	else
		fired = vkeyPressed( input, key.space )
	end
	if fired then
		player_fire( ship )
	end
	ship.cooldown = ship.cooldown - dt

	-- Physics
	ship_v = Vector( 0.0, 0.0, ship.speed, 0.0 )
	world_v = vtransformVector( ship.transform, ship_v )
	vphysic_setVelocity( ship.physic, world_v )
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
function tick( dt )
	if starting then
		starting = false
		start()
	end

	playership_tick( player_ship, dt )

	debug_tick()

	timers_tick( dt )

	update_spawns( player_ship )

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


function spawn_index( pos )
	return math.floor( ( pos - spawn_offset ) / spawn_interval )
end

function spawn_pos( i )
	return i * spawn_interval + spawn_offset;
end

function spawn_target( v )
	vprint( "Spawn_target, v = " .. v )
	local u = 0.0
	local x, y, z = vcanyon_position( u, v )
	local target = gameobject_create( "dat/model/target.s" )
	local position = Vector( x, y + 10.0, z, 1.0 )
	vtransform_setWorldPosition( target.transform, position )
	vbody_registerCollisionCallback( target.body, target_collisionHandler )
	vbody_setLayers( target.body, collision_layer_enemy )
	vbody_setCollidableLayers( target.body, collision_layer_player )
end

function target_collisionHandler( target, collider )
	spawn_explosion( target.transform )
	gameobject_destroy( target )
end

function spawn_atCanyon( u, v, model )
	local x, y, z = vcanyon_position( u, v )
	local position = Vector( x, y, z, 1.0 )
	local obj = gameobject_create( model )
	vtransform_setWorldPosition( obj.transform, position )
end

-- spawn properties
spawn_offset = 0.0
spawn_interval = 0.3
spawn_distance = -300.0
-- spawn tracking
last_spawn = 0.0


function contains( value, range_a, range_b )
	range_max = math.max( range_a, range_b )
	range_min = math.min( range_a, range_b )
	return ( value < range_max ) and ( value >= range_min )
end

-- Spawn all entities in the given range
function entities_spawnAll( near, far )
	canyon_v_scale = 250.0
	near = near / canyon_v_scale
	far = far / canyon_v_scale
	i = spawn_index( near )
	--vprint( "Evaluating spawn index: " .. i )
	spawn_v = i * spawn_interval
	--vprint( "spawn_v " .. spawn_v .. ", near " .. near .. ", far " .. far )
	while contains( spawn_v, near, far ) do
		vprint( "Spawning at " .. spawn_v .. "!" )
		spawn_target( spawn_v )
		i = i + 1
		spawn_v = i * spawn_interval
		--vprint( "Evaluating spawn index: " .. i )
	end
end

-- Spawn all entities that need to be spawned this frame
function update_spawns( ship )
	ship_pos = vtransform_getWorldPosition( ship.transform )
	x,y,z,w = vvector_values( ship_pos )
	far = z + spawn_distance
	entities_spawnAll( last_spawn, far )
	last_spawn = far;
end
