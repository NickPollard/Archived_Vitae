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
uniform sampler2D tex;
uniform sampler2D tex_b;
uniform sampler2D tex_lookup;
uniform vec4 fog_color;
uniform vec4 camera_space_sun_direction;
uniform vec4 viewspace_up;
uniform vec4 directional_light_direction;

// Test Light values
vec4 light_ambient = vec4( 0.4, 0.4, 0.4, 1.0 );
// Directional Light
vec4 directional_light_diffuse = vec4( 1.0, 1.0, 0.8, 1.0 );
vec4 directional_light_specular = vec4( 0.5, 0.5, 0.5, 1.0 );
vec4 sun_color = vec4( 1.0, 0.5, 0.0, 0.0 );

float diffuse_warp( float diffuse ) {
	//return diffuse * 0.5 + 0.5;
	return diffuse;
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
		// TODO: Can this be done in the vertex shader?
		// how does dot( -light_direction, cameraSpace_frag_normal ) vary accross a poly?
		float diffuse = max( 0.0, dot( -directional_light_direction, cameraSpace_frag_normal ));
		total_light_color += directional_light_diffuse * diffuse_warp( diffuse );
		
		// Specular
		vec4 spec_bounce = reflect( directional_light_direction, cameraSpace_frag_normal );
		float spec = max( 0.0, dot( spec_bounce, -view_direction ));
		float shininess = 1.0;
		total_light_color += directional_light_specular * pow( spec, shininess );
	}

	/*
	vec4 cliff_tex_color = texture2D( tex_b, texcoord );
	vec4 flat_color = texture2D( tex, texcoord ) * vert_color;
	vec4 cliff_color = cliff_tex_color * vec4( 0.2, 0.3, 0.6, 1.0 );
	vec4 edge_color = vec4( 0.4, 0.55, 0.8, 1.0 );
	*/
	//vec4 fragColor = total_light_color * mix( flat_color, mix( edge_color, cliff_color, smoothstep( edge, edge_cliff, d )), smoothstep( edge_snow, edge, d ));
	//vec4 fragColor = total_light_color * mix( flat_color, cliff_color, d );

	float edge_snow = 0.03;
	float edge = 0.08;
	float edge_cliff = 0.15;

	float d = 1.0 - clamp( dot( cameraSpace_frag_normal, viewspace_up ), 0.0, 1.0 );
	float cliff = smoothstep( edge_snow, edge_cliff, d );

	vec4 tex_color = mix( texture2D( tex, texcoord ), texture2D( tex_b, texcoord ), cliff );
	vec4 fragColor = total_light_color * texture2D( tex_lookup, vec2( vert_color.x, cliff )) * tex_color;

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
