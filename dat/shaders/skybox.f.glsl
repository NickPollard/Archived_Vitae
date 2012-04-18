//#version 110
// Phong Fragment Shader
#ifdef GL_ES
precision mediump float;
#endif

// Varying
varying vec2 texcoord;
varying float fog;
//varying float height;
//varying vec4 frag_position;

// Uniform
uniform sampler2D tex;
uniform vec4 fog_color;
uniform vec4 sky_color_top;
uniform vec4 sky_color_bottom;

void main() {
#if 0
	gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
#else
	// light-invariant calculations
	vec4 material_diffuse = texture2D( tex, texcoord );
	
	vec4 fragColor = ( sky_color_top * material_diffuse.z )
					+ ( sky_color_bottom * material_diffuse.x );
	fragColor.w = 1.0;
	//gl_FragColor = vec4( fragColor.xyz, 1.0 );

	gl_FragColor = mix( fragColor, fog_color, fog );

	//vec4 sky_color_top = vec4( 0.3, 0.6, 1.0, 1.0 );
	//vec4 sky_color_bottom = vec4( 1.0, 0.4, 0.0, 1.0 );
	//gl_FragColor = vec4( top, bottom, 0.0, 1.0 );
	//gl_FragColor = mix( sky_color_bottom, sky_color_top, height );
#endif
}

