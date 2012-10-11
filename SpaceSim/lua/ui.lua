local ui = {}

function ui.show_splash()
	local color = Vector( 1.0, 1.0, 1.0, 1.0 )
	local splash = vuiPanel_create( engine, "dat/img/splash_vitruvian.tga", color, 0, 0, 1280, 720 )
	return splash
end

function ui.hide_splash( splash )
	vprint( "hide splash" )
	vuiPanel_hide( engine, splash )
end

function ui.show_crosshair()
	-- Crosshair
	local w = 128
	local h = 128
	local x = 640 - ( w / 2 )
	local y = 360 - ( h / 2 ) - 40
	local color = Vector( 0.3, 0.6, 1.0, 0.8 )
	vuiPanel_create( engine, "dat/img/crosshair_arrows.tga", color, x, y, w, h )
end


return ui
