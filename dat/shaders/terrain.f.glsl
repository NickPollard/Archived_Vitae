#ifdef GL_ES
precision mediump float;
#endif
//#version 110

// Varying
varying vec4 frag_position;
varying vec4 cameraSpace_frag_normal;
varying vec2 texcoord;
varying vec4 vert_color;
varying float fog;
varying float cliff;

// Uniform
uniform mat4 modelview;
uniform sampler2D tex;
uniform sampler2D tex_b;
uniform vec4 fog_color;
uniform vec4 camera_space_sun_direction;

// Test Light values
vec4 light_ambient = vec4( 0.4, 0.4, 0.4, 1.0 );
// Directional Light
vec4 directional_light_direction = vec4( 1.0, -0.5, 1.0, 0.0 );
vec4 directional_light_diffuse = vec4( 1.0, 1.0, 0.8, 1.0 );
vec4 directional_light_specular = vec4( 0.5, 0.5, 0.5, 1.0 );
vec4 sun_color = vec4( 1.0, 0.5, 0.0, 0.0 );

float diffuse_scale = 0.5;
float diffuse_bias = 0.5;

float diffuse_warp( float diffuse ) {
	return diffuse * diffuse_scale + diffuse_bias;
}

float sun_fog( vec4 local_sun_dir, vec4 fragment_position ) {
	return max( 0.0, dot( local_sun_dir, normalize( fragment_position )));
}

void main() {
#if 1
	// light-invariant calculations
	vec4 view_direction = normalize( frag_position );
	vec4 total_light_color = light_ambient;

	// Directional light
	{
		// Ambient + Diffuse
		// TODO: This is constant, can be calculated once
		vec4 light_direction = normalize( modelview * directional_light_direction );

		// TODO: Can this be done in the vertex shader?
		// how does dot( -light_direction, cameraSpace_frag_normal ) vary accross a poly?
		float diffuse = max( 0.0, dot( -light_direction, cameraSpace_frag_normal ));
		total_light_color += directional_light_diffuse * diffuse_warp( diffuse );
		
		// Specular
		vec4 spec_bounce = reflect( light_direction, cameraSpace_frag_normal );
		float spec = max( 0.0, dot( spec_bounce, -view_direction ));
		float shininess = 1.0;
		total_light_color += directional_light_specular * pow( spec, shininess );
	}

	vec4 cliff_tex_color = texture2D( tex_b, texcoord );
	vec4 flat_color = /* texture2D( tex, texcoord ) * */ vec4( 0.8, 0.9, 1.0, 1.0 );
	vec4 cliff_color = /* cliff_tex_color * */ vec4( 0.2, 0.3, 0.6, 1.0 );
	vec4 edge_color = vec4( 0.4, 0.55, 0.8, 1.0 ) /* * cliff_tex_color */;
	/*
	vec4 flat_color = texture2D( tex, texcoord ) * vec4( 0.7, 0.3, 0.2, 1.0 );
	vec4 cliff_color = cliff_tex_color * vec4( 0.3, 0.1, 0.1, 1.0 );
	vec4 edge_color = vec4( 0.6, 0.2, 0.2, 1.0 ) * cliff_tex_color;
*/	

	float edge_snow = 0.03;
	float edge = 0.08;
	float edge_cliff = 0.15;
	float d = 1.0 - clamp( dot( cameraSpace_frag_normal, modelview * vec4( 0.0, 1.0, 0.0, 0.0 )), 0.0, 1.0 );
	vec4 fragColor = total_light_color * mix( flat_color, mix( edge_color, cliff_color, smoothstep( edge, edge_cliff, d )), smoothstep( edge_snow, edge, d ));

	// sunlight on fog
	float fog_sun_factor = sun_fog( camera_space_sun_direction, frag_position );
	vec4 local_fog_color = fog_color + (sun_color * fog_sun_factor);

	gl_FragColor = mix( fragColor, local_fog_color, fog );
	gl_FragColor.w = 1.0;

	//gl_FragColor = cameraSpace_frag_normal;

#else
	gl_FragColor = vec4( 1.0, 1.0, 0.5, 1.0 );
#endif

}
