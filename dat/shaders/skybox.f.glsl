//#version 110
// Phong Fragment Shader
#ifdef GL_ES
precision mediump float;
#endif

// Varying
varying vec4 frag_position;
varying vec2 texcoord;
varying float height;

// Uniform
uniform sampler2D tex;
uniform vec4 fog_color;

void main() {
#if 0
	gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
#else
	// light-invariant calculations
	vec4 material_diffuse = texture2D( tex, texcoord );
	
	vec4 color_top = vec4( 0.3, 0.6, 1.0, 1.0 );
	vec4 color_bottom = vec4( 1.0, 0.4, 0.0, 1.0 );

	vec4 fragColor = ( color_top * material_diffuse.z )
					+ ( color_bottom * material_diffuse.x );
	fragColor.w = 1.0;
	//gl_FragColor = vec4( fragColor.xyz, 1.0 );

	//vec4 fog_color = vec4( 1.0, 0.6, 0.2, 1.0 );
	float fog_far = 350.0;
	float fog_near = 100.0;
	float fog_height = 160.0;
	float height_factor = clamp( ( fog_height - (height * 500.0)) / fog_height, 0.0, 1.0 );
	float fog = clamp( ( frag_position.z - fog_near ) / ( fog_far - fog_near ), 0.0, 1.0 ) * height_factor;

	gl_FragColor = mix( fragColor, fog_color, fog );

//	gl_FragColor = vec4( top, bottom, 0.0, 1.0 );
	//gl_FragColor = mix( color_bottom, color_top, height );
//	gl_FragColor = material_diffuse;
#endif
}

