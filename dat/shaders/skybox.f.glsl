//#version 110
// Phong Fragment Shader
#ifdef GL_ES
precision mediump float;
#endif

// Varying
varying vec2 texcoord;
varying float fog;
//varying float height;
varying vec4 frag_position;

// Uniform
uniform sampler2D tex;
uniform vec4 fog_color;
uniform vec4 sky_color_top;
uniform vec4 sky_color_bottom;
uniform mat4 modelview;

const vec4 sun_color = vec4( 1.0, 0.5, 0.0, 1.0 );
const vec4 sun_dir = vec4( 0.0, 0.0, 1.0, 0.0 );

float sun_fog( vec4 sun_direction, vec4 fragment_position, mat4 modelview_mat ) {
	vec4 local_sun_dir = modelview_mat * sun_direction;
	return max( 0.0, dot( local_sun_dir, normalize( fragment_position )));
}

void main() {
#if 0
	gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
#else
	// light-invariant calculations
	vec4 material_diffuse = texture2D( tex, texcoord );

/*	
	vec4 fragColor = ( sky_color_top * material_diffuse.z )
					+ ( sky_color_bottom * material_diffuse.x );
	fragColor.w = 1.0;
	*/
	//gl_FragColor = vec4( fragColor.xyz, 1.0 );

	float brightness = min( sky_color_top.x +
						sky_color_top.y +
						sky_color_top.z, 
						1.0 );

	vec4 cloud_color = vec4( brightness, brightness, brightness, 1.0 );

	// color = top * blue
	// then blend in cloud (white * green, blend alpha )
	vec4 fragColor = mix( sky_color_top * material_diffuse.z, cloud_color * material_diffuse.y, material_diffuse.w );
	// then add bottom * red
	fragColor = fragColor + sky_color_bottom * material_diffuse.x;
	fragColor.w = 1.0;

	// sunlight on fog
	float fog_sun_factor = sun_fog( sun_dir, frag_position, modelview );
	vec4 local_fog_color = mix( fog_color, sun_color, fog_sun_factor );

	gl_FragColor = mix( fragColor, local_fog_color, fog );

	//vec4 sky_color_top = vec4( 0.3, 0.6, 1.0, 1.0 );
	//vec4 sky_color_bottom = vec4( 1.0, 0.4, 0.0, 1.0 );
	//gl_FragColor = vec4( top, bottom, 0.0, 1.0 );
	//gl_FragColor = mix( sky_color_bottom, sky_color_top, height );
#endif
}

