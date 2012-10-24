local fx = {}

function fx.spawn_missile_explosion( position )
	local t = vcreateTransform()
	vparticle_create( engine, t, "dat/script/lisp/missile_explosion.s" )
	vtransform_setWorldSpaceByTransform( t, position )
end

function fx.spawn_explosion( position )
	local t = vcreateTransform()
	vparticle_create( engine, t, "dat/script/lisp/explosion.s" )
	vparticle_create( engine, t, "dat/script/lisp/explosion_b.s" )
	vparticle_create( engine, t, "dat/script/lisp/explosion_c.s" )
	vtransform_setWorldSpaceByTransform( t, position )
end

return fx
