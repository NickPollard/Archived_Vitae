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
uniform vec4 camera_space_sun_direction;
//const vec4 camera_space_sun_direction = vec4( 0.0, 0.0, 1.0, 0.0 );

const vec4 sun_color = vec4( 1.0, 0.5, 0.0, 1.0 );
const vec4 cloud_color = vec4( 1.0, 1.0, 1.0, 1.0 );

float sun_fog( vec4 local_sun_dir, vec4 fragment_position ) {
	return max( 0.0, dot( local_sun_dir, normalize( fragment_position )));
}

void main() {
#if 0
	gl_FragColor = vec4( 0.0, 0.0, 1.0, 1.0 );
#else
	// light-invariant calculations
	vec4 material_diffuse = texture2D( tex, texcoord );

	// color = top * blue
	// then blend in cloud (white * green, blend alpha )
	vec4 fragColor = mix( sky_color_top * material_diffuse.z, cloud_color * material_diffuse.y, material_diffuse.w );
	// then add bottom * red
	fragColor = fragColor + sky_color_bottom * material_diffuse.x;
	fragColor.w = 1.0;

	// sunlight on fog
#if 1
	vec4 local_fog_color = mix( fog_color, sun_color, sun_fog( camera_space_sun_direction, frag_position ));
#else
	vec4 local_fog_color = fog_color;
#endif
	
	gl_FragColor = mix( fragColor, local_fog_color, fog );
#endif
}

