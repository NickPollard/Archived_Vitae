-- SpaceSim main game lua script

--[[

This file contains the main game routines for the SpaceSim game.
Most game logic is written in Lua, whilst the core numerical processing (rendering, physics, animation etc.)
are handled by the Vitae engine in C.

Lua should be able to do everything C can, but where performance is necessary, code should be rewritten in
C and only controlled remotely by Lua

]]--

-- Debug settings
debug_spawning_enabled = true

-- Load Modules
	package.path = "./SpaceSim/lua/?.lua"
	ai = require "ai"
	fx = require "fx"
	ui = require "ui"

-- player - this object contains general data about the player
player = nil
-- player_ship - the actual ship entity flying around
player_ship = nil

-- Create a spacesim Game object
-- A gameobject has a visual representation (model), a physical entity for velocity and momentum (physic)
-- and a transform for locating it in space (transform)
function gameobject_create( model_file )
	local g = {}
	g.model = vcreateModelInstance( model_file )
	g.physic = vcreatePhysic()
	g.transform = vcreateTransform()
	g.body = vcreateBodySphere( g )
	--g.body = vcreateBodyMesh( g, g.model )
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
	inTime( 0.2, function() gameobject_delete( g ) end )

	if g.body then
		vdestroyBody( g.body )
		g.body = nil
	end
end

function gameobject_delete( g )
	if g.model then
		vdeleteModelInstance( g.model )
		g.model = nil
	end
	if g.transform then
		vdestroyTransform( scene, g.transform )
		g.transform = nil
	end
	if g.physic then
		vphysic_destroy( g.physic )
		g.physic = nil
	end
	if g.body then
		vdestroyBody( g.body )
		g.body = nil
	end
end

projectile_model = "dat/model/missile.s"
weapons_cooldown = 0.15

function player_fire( ship )
	if ship.cooldown <= 0.0 then
		muzzle_position = Vector( 1.2, 0.0, 0.0, 1.0 );
		fire_missile( ship, muzzle_position, projectile_model, player_bullet_speed );
		muzzle_position = Vector( -1.2, 0.0, 0.0, 1.0 );
		fire_missile( ship, muzzle_position, projectile_model, player_bullet_speed );
		ship.cooldown = weapons_cooldown
	end
end

function missile_destroy( missile )
	gameobject_destroy( missile )
	if missile.glow then
		vparticle_destroy( missile.glow )
	end
	if missile.trail then
		vparticle_destroy( missile.trail )
	end
end

function missile_collisionHandler( missile, other )
	fx.spawn_missile_explosion( missile.transform )
	vphysic_setVelocity( missile.physic, Vector( 0.0, 0.0, 0.0, 0.0 ))
	missile_destroy( missile )
end

missiles		= { count = 0 }
turrets			= { count = 0 }
interceptors	= { count = 0 }
player_bullet_speed		= 250.0;
enemy_bullet_speed		= 150.0;
homing_missile_speed	= 50.0;

function setCollision_playerBullet( object )
	vbody_setLayers( object.body, collision_layer_bullet )
	vbody_setCollidableLayers( object.body, collision_layer_enemy )
end

function setCollision_enemyBullet( object )
	vbody_setLayers( object.body, collision_layer_bullet )
	vbody_setCollidableLayers( object.body, collision_layer_player )
end

function fire_missile( source, offset, model, speed )
	-- Create a new Projectile
	local projectile = gameobject_create( model )
	projectile.tick = nil

	setCollision_playerBullet( projectile )
	vbody_registerCollisionCallback( projectile.body, missile_collisionHandler )

	-- Position it at the correct muzzle position and rotation
	muzzle_world_pos = vtransformVector( source.transform, offset )
	vtransform_setWorldSpaceByTransform( projectile.transform, source.transform )
	vtransform_setWorldPosition( projectile.transform, muzzle_world_pos )

	-- Attach a particle effect to the object
	projectile.glow = vparticle_create( engine, projectile.transform, "dat/script/lisp/bullet.s" )

	-- Apply initial velocity
	source_velocity = Vector( 0.0, 0.0, speed, 0.0 )
	world_v = vtransformVector( source.transform, source_velocity )
	vphysic_setVelocity( projectile.physic, world_v );

	-- Queue up delete
	inTime( 2.0, function () missile_destroy( projectile ) end )

	-- Store the projectile so it doesn't get garbage collected
	array_add( missiles, projectile )
end

function fire_enemy_missile( source, offset, model, speed )
	-- Create a new Projectile
	local projectile = gameobject_create( model )
	projectile.tick = nil

	setCollision_enemyBullet( projectile )
	vbody_registerCollisionCallback( projectile.body, missile_collisionHandler )

	-- Position it at the correct muzzle position and rotation
	muzzle_world_pos = vtransformVector( source.transform, offset )
	vtransform_setWorldSpaceByTransform( projectile.transform, source.transform )
	vtransform_setWorldPosition( projectile.transform, muzzle_world_pos )

	-- Attach a particle effect to the object
	projectile.glow = vparticle_create( engine, projectile.transform, "dat/script/lisp/bullet.s" )

	-- Apply initial velocity
	source_velocity = Vector( 0.0, 0.0, speed, 0.0 )
	world_v = vtransformVector( source.transform, source_velocity )
	vphysic_setVelocity( projectile.physic, world_v );

	-- Queue up delete
	inTime( 2.0, function () missile_destroy( projectile ) end )

	array_add( missiles, projectile )
end

homing_missile_turn_angle_per_second = math.pi / 2
function homing_missile_tick( target_transform )
	return function ( missile, dt )
		if missile.physic and missile.transform then
			local current_position = vtransform_getWorldPosition( missile.transform )
			local target_position = vtransform_getWorldPosition( target_transform )
			local target_direction = vvector_normalize( vvector_subtract( target_position, current_position ))
			local current_dir = vquaternion_fromTransform( missile.transform )
			local target_dir = vquaternion_look( target_direction )
			--local new_dir = vquaternion_slerp( current_dir, target_dir, 1.0 * dt )
			local new_dir = vquaternion_slerpAngle( current_dir, target_dir, homing_missile_turn_angle_per_second * dt )
			local world_velocity = vquaternion_rotation( new_dir, Vector( 0.0, 0.0, homing_missile_speed, 0.0 ))
			vphysic_setVelocity( missile.physic, world_velocity )
			vtransform_setRotation( missile.transform, new_dir )
		end
	end
end

function fire_enemy_homing_missile( source, offset, model, speed )
	-- Create a new Projectile
	local projectile = gameobject_create( model )
	setCollision_enemyBullet( projectile )

	vbody_registerCollisionCallback( projectile.body, missile_collisionHandler )

	-- Position it at the correct muzzle position and rotation
	muzzle_world_pos = vtransformVector( source.transform, offset )
	vtransform_setWorldSpaceByTransform( projectile.transform, source.transform )
	vtransform_setWorldPosition( projectile.transform, muzzle_world_pos )

	-- Attach a particle effect to the object
	projectile.glow = vparticle_create( engine, projectile.transform, "dat/script/lisp/red_bullet.s" )
	projectile.trail = vparticle_create( engine, projectile.transform, "dat/script/lisp/red_trail.s" )

	-- Apply initial velocity
	source_velocity = Vector( 0.0, 0.0, speed, 0.0 )
	world_v = vtransformVector( source.transform, source_velocity )
	vphysic_setVelocity( projectile.physic, world_v );

	-- Queue up delete
	inTime( 5.0, function () missile_destroy( projectile ) end )

	-- Store the projectile so it doesn't get garbage collected
	array_add( missiles, projectile )
	projectile.tick = homing_missile_tick( player_ship.transform )
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
	p.yaw = 0
	p.pitch = 0
	p.roll = 0
	p.camera_transform = vcreateTransform()
	
	-- Init Collision
	vbody_registerCollisionCallback( p.body, player_ship_collisionHandler )
	vbody_setLayers( p.body, collision_layer_player )
	vbody_setCollidableLayers( p.body, collision_layer_enemy )

	return p
end

function playership_addEngineGlows( p )
	local engine_trail = "dat/script/lisp/engine_trail.s"
	local engine_glow = "dat/script/lisp/engine_glow.s"

	local t_a = vcreateTransform( p.transform )
	local offset = Vector( 4.5, -0.1, -1.2, 0.0 )
	vtransform_setLocalPosition( t_a, offset )
	
	local t_b = vcreateTransform( p.transform )
	local offset = Vector( -4.5, -0.1, -1.2, 0.0 )
	vtransform_setLocalPosition( t_b, offset )
	
	local t_c = vcreateTransform( p.transform )
	local offset = Vector( 0.2, 0.7, -1.9, 0.0 )
	vtransform_setLocalPosition( t_c, offset )
	
	local t_d = vcreateTransform( p.transform )
	local offset = Vector( -0.2, 0.7, -1.9, 0.0 )
	vtransform_setLocalPosition( t_d, offset )

	p.engine_trail_a = vparticle_create( engine, t_a, engine_trail )
	p.engine_trail_b = vparticle_create( engine, t_b, engine_trail )
	p.engine_trail_c = vparticle_create( engine, t_c, engine_trail )
	p.engine_trail_d = vparticle_create( engine, t_d, engine_trail )
	p.engine_glow_a = vparticle_create( engine, t_a, engine_glow )
	p.engine_glow_b = vparticle_create( engine, t_b, engine_glow )
	p.engine_glow_c = vparticle_create( engine, t_c, engine_glow )
	p.engine_glow_d = vparticle_create( engine, t_d, engine_glow )
end

starting = true

-- Set up the Lua State
function init()
	vprint( "init" )
	starting = true
	color = Vector( 1.0, 1.0, 1.0, 1.0 )
	local vignette = vuiPanel_create( engine, "dat/img/vignette.tga", color, 0, 360, 1280, 360 )
	
	splash_intro()

	library, msg = loadfile( "SpaceSim/lua/library.lua" )
	library()
end

camera = "chase"
flycam = nil
chasecam = nil

function vrand( lower, upper )
	return math.random() * ( upper - lower ) + lower
end

function ship_collisionHandler( ship, collider )
	fx.spawn_explosion( ship.transform );
	ship_destroy( ship )
	ship.behaviour = ai.dead
end

function ship_destroy( ship )
	gameobject_destroy( ship )
end

function ship_delete( ship )
	gameobject_delete( ship )
	ship.behaviour = ai.dead
end

function setup_controls()
	-- Set up steering input for the player ship
	if touch_enabled then
		-- Steering
		local w = 720
		local h = 720
		local x = 1280 - w
		local y = 720 - h
		player_ship.joypad_mapper = drag_map()
		player_ship.joypad = vcreateTouchPad( input, x, y, w, h )
		player_ship.steering_input = steering_input_drag
		-- UI drawing is upside down compared to touchpad placement - what to do about this?

		-- Firing Trigger
		local x = 0
		local y = 0
		local w = 1280 - 720
		local h = 720
		player_ship.fire_trigger = vcreateTouchPad( input, x, y, w, h )
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
	fx.spawn_explosion( ship.transform )

	-- not using gameobject_destroy as we need to sync transform dying with camera rejig
	inTime( 0.2, function () vdeleteModelInstance( ship.model ) 
							vprint( "Destroying ship physic" )
							vphysic_destroy( ship.physic )
							ship.physic = nil
				end )
	vdestroyBody( ship.body )

	-- destroy engine glows
	vparticle_destroy( ship.engine_glow_a )
	vparticle_destroy( ship.engine_glow_b )
	vparticle_destroy( ship.engine_glow_c )
	vparticle_destroy( ship.engine_glow_d )
	vparticle_destroy( ship.engine_trail_a )
	vparticle_destroy( ship.engine_trail_b )
	vparticle_destroy( ship.engine_trail_c )
	vparticle_destroy( ship.engine_trail_d )

	-- queue a restart
	inTime( 2.0, function ()
		vprint( "Restarting" )
		vdestroyTransform( scene, ship.transform )
		restart() 
		gameplay_start()
	end )
end

collision_layer_player = 1
collision_layer_enemy = 2
collision_layer_bullet = 3

function gameplay_start()
	player_active = true
	inTime( 2.0, function () 
		player_ship.speed = 30.0 
		playership_addEngineGlows( player_ship )
		if debug_spawning_enabled then
			spawning_active = true
		end
		already_spawned = 0.0
	end )
end


function restart()
	spawning_active = false
	player_active = false
	entities_despawnAll()
	-- We create a player object which is a game-specific Lua class
	-- The player class itself creates several native C classes in the engine
	player_ship = playership_create()

	-- Init position
	local start_position = Vector( 0.0, 0.0, 20.0, 1.0 ) 
	vtransform_setWorldPosition( player_ship.transform, start_position )

	-- Init velocity
	player_ship.speed = 0.0
	local no_velocity = Vector( 0.0, 0.0, player_ship.speed, 0.0 )
	vphysic_setVelocity( player_ship.physic, no_velocity )

	chasecam = vchasecam_follow( engine, player_ship.camera_transform )
	flycam = vflycam( engine )
	vscene_setCamera( chasecam )
	setup_controls()
end

function loadParticles( )
	local t = vcreateTransform()
	local particle
	particle = vparticle_create( engine, t, "dat/script/lisp/missile_explosion.s" )
	vparticle_destroy( particle )
	particle = vparticle_create( engine, t, "dat/script/lisp/explosion.s" )
	vparticle_destroy( particle )
	particle = vparticle_create( engine, t, "dat/script/lisp/explosion_b.s" )
	vparticle_destroy( particle )
	particle = vparticle_create( engine, t, "dat/script/lisp/explosion_c.s" )
	vparticle_destroy( particle )
	particle = vparticle_create( engine, t, "dat/script/lisp/bullet.s" )
	vparticle_destroy( particle )
	vmodel_preload( projectile_model )
end

function testSpawns()
	spawn_v = -3.0
	local width = 25.0
	while spawn_v < 3.0 do
		spawn_v = spawn_v + 0.3
		spawn_atCanyon( -width, spawn_v, "dat/model/skyscraper.s" )
		spawn_atCanyon( width, spawn_v, "dat/model/skyscraper.s" )
	end
end

function makefunction( text )
	local a = nil
	a = function() vprint( text ) return a end
	return a
end

function splash_intro()
	vtexture_preload( "dat/img/splash_author.tga" )
	local studio_splash = ui.show_splash( "dat/img/splash_vitruvian.tga" )
	inTime( 2.0, function () 
		ui.hide_splash( studio_splash ) 
		local author_splash = ui.show_splash( "dat/img/splash_author.tga" )
		inTime( 2.0, function ()
			ui.hide_splash( author_splash ) 
			ui.show_crosshair()
			gameplay_start()
		end )
	end )
end

function start()
	loadParticles()

	restart()
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

function playership_weaponsTick( ship, dt )
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
end

function clamp( min, max, value )
	return math.min( max, math.max( min, value ))
end

function lerp( a, b, k )
	return a + ( b - a ) * k
end

function playership_tick( ship, dt )
	yaw_per_second = 1.5 
	pitch_per_second = 1.5

	local input_yaw = 0.0
	local input_pitch = 0.0
	input_yaw, input_pitch = ship.steering_input()

	-- set to -1.0 to invert
	invert_pitch = 1.0
	pitch = invert_pitch * input_pitch * pitch_per_second * dt;
	yaw = input_yaw * yaw_per_second * dt;

	ship.yaw = ship.yaw + yaw
	ship.pitch = ship.pitch + pitch
	target_roll = yaw * -60.0;
	max_roll = 1.0
	max_roll_delta = 2.0 * dt
	delta = target_roll - ship.roll
	roll_delta = clamp( -max_roll_delta, max_roll_delta, delta )
	roll = ship.roll + roll_delta
	ship.roll = clamp( -max_roll, max_roll, roll )
	
	vtransform_eulerAngles( ship.transform, ship.yaw, ship.pitch, ship.roll )
	-- Camera transform shares ship position and yaw, pitch; but not roll
	vtransform_setWorldSpaceByTransform( ship.camera_transform, ship.transform )
	vtransform_eulerAngles( ship.camera_transform, ship.yaw, ship.pitch, 0.0 )

	-- throttle
	width = 100
	acceleration = 1.0
	delta_speed = acceleration * dt;
	ship.speed = ship.speed + delta_speed

	playership_weaponsTick( ship, dt )

	-- Physics
	world_v = vtransformVector( ship.transform, Vector( 0.0, 0.0, ship.speed, 0.0 ))
	if ship.physic then
		vphysic_setVelocity( ship.physic, world_v )
	end
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
end

spawning_active = false

-- Called once per frame to update the current Lua State
function tick( dt )
	if starting then
		starting = false
		start()
	end

	--sky_tick( chasecam, dt )

	if player_active then
		playership_tick( player_ship, dt )
	end

	debug_tick()

	timers_tick( dt )

	if spawning_active then
		update_spawns( player_ship )
		update_despawns( player_ship )
	end

	tick_array( turrets, dt )
	tick_array( interceptors, dt )
	tick_array( missiles, dt )
end

-- Called on termination to clean up after itself
function terminate()
	player = nil
end

function delay( time, command )
	if time <= 0 then
		command()
	else
		print( string.format( "Delay timer: %d", time ))
		delay( time-1, command )
	end
end

function spawn_index( pos )
	return math.floor( ( pos - spawn_offset ) / spawn_interval )
end

function spawn_pos( i )
	return i * spawn_interval + spawn_offset;
end



turret_cooldown = 0.4

function turret_fire( turret )
	fire_enemy_missile( turret, Vector(  4.0, 6.0, 0.0, 1.0), projectile_model, enemy_bullet_speed )
	fire_enemy_missile( turret, Vector( -4.0, 6.0, 0.0, 1.0), projectile_model, enemy_bullet_speed )
end

function turret_tick( turret, dt )
	turret.state = turret.state( turret, dt )
end

function tick_array( array, dt )
	for element in iterator( array ) do
		if element.tick then
			element.tick( element, dt )
		end
	end
end

function spawn_turret( u, v )
	local spawn_height = 20.0
	vprint( "Spawn_turret, v = " .. v )

	-- position
	local x, y, z = vcanyon_position( u, v )
	local position = Vector( x, y + spawn_height, z, 1.0 )
	local turret = gameobject_create( "dat/model/gun_turret.s" )
	vtransform_setWorldPosition( turret.transform, position )

	-- Orientation
	local facing_x, facing_y, facing_z = vcanyon_position( u, v - ( 1.0 / canyon_v_scale ))
	local facing_position = Vector( facing_x, y + spawn_height, facing_z, 1.0 )
	vtransform_facingWorld( turret.transform, facing_position )

	-- Physics
	vbody_registerCollisionCallback( turret.body, turret_collisionHandler )
	vbody_setLayers( turret.body, collision_layer_enemy )
	vbody_setCollidableLayers( turret.body, collision_layer_player )

	turret.tick = turret_tick
	turret.cooldown = turret_cooldown

	-- ai
	turret.state = turret_state_inactive

	turrets.count = turrets.count + 1
	turrets[turrets.count] = turret
end

function turret_collisionHandler( target, collider )
	fx.spawn_explosion( target.transform )
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
spawn_interval = 300.0
spawn_distance = 300.0
despawn_distance = 100.0 -- how far behind to despawn units
-- spawn tracking
already_spawned = 0.0

function contains( value, range_a, range_b )
	range_max = math.max( range_a, range_b )
	range_min = math.min( range_a, range_b )
	return ( value < range_max ) and ( value >= range_min )
end

canyon_v_scale = 1.0

-- Spawn all entities in the given range
function entities_spawnRange( near, far )
	near = near / canyon_v_scale
	far = far / canyon_v_scale
	i = spawn_index( near ) + 1
	spawn_v = i * spawn_interval
	while contains( spawn_v, near, far ) do
			local interceptor_offset_u = 20.0
			if ( i + 1 ) % 3.0 == 0 then
				spawn_interceptor( interceptor_offset_u, spawn_v, interceptor_attack_homing )
				spawn_interceptor( -interceptor_offset_u, spawn_v, interceptor_attack_homing )
			else
				spawn_interceptor( interceptor_offset_u, spawn_v, interceptor_attack_gun )
				spawn_interceptor( -interceptor_offset_u, spawn_v, interceptor_attack_gun )
			end
		i = i + 1
		spawn_v = i * spawn_interval
	end
end

function entities_despawnAll()
	for unit in iterator( interceptors ) do
		vprint( "Despawning interceptor" )
		ship_delete( unit )
		unit.behaviour = ai.dead
	end
end

-- Spawn all entities that need to be spawned this frame
function update_spawns( ship )
	ship_pos = vtransform_getWorldPosition( ship.transform )
	u,v = vcanyon_fromWorld( ship_pos )
	spawn_up_to = v + spawn_distance
	entities_spawnRange( already_spawned, spawn_up_to )
	already_spawned = spawn_up_to;
end

function update_despawns( ship ) 
	ship_pos = vtransform_getWorldPosition( ship.transform )
	u,v = vcanyon_fromWorld( ship_pos )
	despawn_up_to = v - despawn_distance

	for unit in iterator( interceptors ) do
		-- TODO remove them properly
		if unit.transform then
			unit_pos = vtransform_getWorldPosition( unit.transform )
			u,v = vcanyon_fromWorld( unit_pos )
			if v < despawn_up_to then
				vprint( "despawn" )
				ship_delete( unit )
				unit = nil
			end
		end
	end
end

function turret_state_inactive( turret, dt )
	player_close = ( vtransform_distance( player_ship.transform, turret.transform ) < 200.0 )

	if player_close then
		return turret_state_active
	else
		return turret_state_inactive
	end
end

function turret_state_active( turret, dt )
	player_close = ( vtransform_distance( player_ship.transform, turret.transform ) < 200.0 )

	if turret.cooldown < 0.0 then
		turret_fire( turret )
		turret.cooldown = turret_cooldown
	end
	turret.cooldown = turret.cooldown - dt

	if player_close then
		return turret_state_active
	else
		return turret_state_inactive
	end
end

function entity_setSpeed( entity, speed )
	entity.speed = speed
	local entity_velocity = Vector( 0.0, 0.0, entity.speed, 0.0 )
	local world_velocity = vtransformVector( entity.transform, entity_velocity )
	vphysic_setVelocity( entity.physic, world_velocity )
end

-- Interceptor Stats
interceptor_min_speed = 3.0
interceptor_speed = 160.0
interceptor_weapon_cooldown = 0.4
homing_missile_cooldown = 1.2

function entity_moveTo( x, y, z )
	return function ( entity, dt )
		entity_setSpeed( entity, interceptor_speed )
		local facing_position = Vector( x, y, z, 1.0 )
		vdebugdraw_cross( facing_position, 10.0 )
		vtransform_facingWorld( entity.transform, facing_position )
	end
end

function entity_strafeTo( target_x, target_y, target_z, facing_x, facing_y, facing_z )
	return function ( entity, dt )
		-- Move to correct position
		local target_position = Vector( target_x, target_y, target_z, 1.0 )
		local current_position = vtransform_getWorldPosition( entity.transform )
		local distance_remaining = vvector_distance( target_position, current_position )
		local speed = clamp( interceptor_min_speed, interceptor_speed, distance_remaining)

		local world_direction = vvector_normalize( vvector_subtract( target_position, current_position ))
		local world_velocity = vvector_scale( world_direction, speed )
		vphysic_setVelocity( entity.physic, world_velocity )

		-- Face correct direction
		local facing_position = Vector( facing_x, facing_y, facing_z, 1.0 )
		vtransform_facingWorld( entity.transform, facing_position )
		
		--vdebugdraw_cross( target_position, 10.0 )
	end
end

function interceptor_attack_gun( x, y, z )
	return function ( interceptor, dt )
		local facing_position = Vector( x, y, z, 1.0 )
		vtransform_facingWorld( interceptor.transform, facing_position )
		entity_setSpeed( interceptor, 0.0 )

		if interceptor.cooldown < 0.0 then
			interceptor_fire( interceptor )
			interceptor.cooldown = interceptor_weapon_cooldown
		end
		interceptor.cooldown = interceptor.cooldown - dt
	end
end

function interceptor_attack_homing( x, y, z )
	return function ( interceptor, dt )
		local facing_position = Vector( x, y, z, 1.0 )
		vtransform_facingWorld( interceptor.transform, facing_position )
		entity_setSpeed( interceptor, 0.0 )

		if interceptor.cooldown < 0.0 then
			interceptor_fire_homing( interceptor )
			interceptor.cooldown = homing_missile_cooldown
		end
		interceptor.cooldown = interceptor.cooldown - dt
	end
end

function interceptor_fire( interceptor )
	fire_enemy_missile( interceptor, Vector(  4.0, 0.0, 0.0, 1.0 ), projectile_model, enemy_bullet_speed )
	fire_enemy_missile( interceptor, Vector( -4.0, 0.0, 0.0, 1.0 ), projectile_model, enemy_bullet_speed )
end

function interceptor_fire_homing( interceptor )
	fire_enemy_homing_missile( interceptor, Vector( 0.0, 0.0, 0.0, 1.0 ), projectile_model, homing_missile_speed )
end

function entity_atPosition( entity, x, y, z, max_distance )
	local position = Vector( x, y, z, 1.0 )
	local entity_position = vtransform_getWorldPosition( entity.transform )
	local distance = vvector_distance( entity_position, position )
	return distance < max_distance
end

function create_interceptor()
	local interceptor = gameobject_create( "dat/model/ship_hd.s" )
	interceptor.cooldown = 0.0
	
	-- Init Collision
	vbody_registerCollisionCallback( interceptor.body, ship_collisionHandler )
	vbody_setLayers( interceptor.body, collision_layer_enemy )
	vbody_setCollidableLayers( interceptor.body, collision_layer_player )

	-- Activate
	interceptor.tick = interceptor_tick
	array_add( interceptors, interceptor )
	return interceptor
end

function interceptor_behaviour( interceptor, move_to, attack_target, attack_type )
	local enter = nil
	local attack = nil
	enter =		ai.state( entity_strafeTo( move_to.x, move_to.y, move_to.z, attack_target.x, move_to.y, attack_target.z ),		
							function () if entity_atPosition( interceptor, move_to.x, move_to.y, move_to.z, 5.0 ) then 
									return attack 
								else 
									return enter 
								end 
							end )

	attack =	ai.state( attack_type( attack_target.x, attack_target.y, attack_target.z ),						function () return attack end )
	return enter
end

interceptor_spawn_u_offset = -200
interceptor_spawn_v_offset = -200
interceptor_spawn_y_offset = 100
interceptor_target_v_offset = 100

function spawn_interceptor( u, v, attack_type )
	local y_height = 40
	local spawn_x, spawn_y, spawn_z = vcanyon_position( u + interceptor_spawn_u_offset, v + interceptor_spawn_v_offset )
	local spawn_position = Vector( spawn_x, spawn_y + interceptor_spawn_y_offset, spawn_z, 1.0 )
	local x, y, z = vcanyon_position( u, v + interceptor_target_v_offset )
	move_to = { x = x, y = y + y_height, z = z }

	local interceptor = create_interceptor()
	
	vtransform_setWorldPosition( interceptor.transform, spawn_position )
	local x, y, z = vcanyon_position( u, v + interceptor_target_v_offset - 100.0 )
	local attack_target = { x = x, y = move_to.y, z = z }
	interceptor.behaviour = interceptor_behaviour( interceptor, move_to, attack_target, attack_type )
end

function array_add( array, object )
	array.count = array.count + 1
	array[array.count] = object
end

function interceptor_tick( interceptor, dt )
	if interceptor.behaviour then
		interceptor.behaviour = interceptor.behaviour( interceptor, dt )
	end
end

