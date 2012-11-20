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
varying vec4 local_fog_color;
varying float cliff;
varying float specular;

// Uniform
uniform sampler2D tex;
uniform sampler2D tex_b;
uniform sampler2D tex_lookup;
uniform vec4 directional_light_direction;

// Test Light values
vec4 light_ambient = vec4( 0.2, 0.2, 0.2, 1.0 );
// Directional Light
vec4 directional_light_diffuse = vec4( 1.0, 1.0, 0.8, 1.0 );
vec4 directional_light_specular = vec4( 0.5, 0.5, 0.5, 1.0 );

float diffuse_warp( float diffuse ) {
	//return diffuse * 0.5 + 0.5;
	return diffuse;
}

void main() {
#if 1
	// light-invariant calculations
	vec4 total_light_color = light_ambient;

	// Directional light
	{
		// Ambient + Diffuse
		// TODO: Can this be done in the vertex shader?
		// how does dot( -light_direction, cameraSpace_frag_normal ) vary accross a poly?
		float diffuse = max( 0.0, dot( -directional_light_direction, cameraSpace_frag_normal ));
		total_light_color += directional_light_diffuse * diffuse_warp( diffuse );
		
		//float shininess = 1.0;
		//total_light_color += directional_light_specular * pow( specular, shininess );
		// Whilst shininess == 1.0, don't do any pow()
		total_light_color += directional_light_specular * specular;
	}
	vec4 a = texture2D( tex, texcoord );
	vec4 b = texture2D( tex_b, texcoord );
	vec4 tex_color = mix( a, b, cliff );
	vec4 fragColor = total_light_color * texture2D( tex_lookup, vec2( vert_color.x, cliff )) * tex_color;

	gl_FragColor = mix( fragColor, local_fog_color, fog );
	gl_FragColor.w = 1.0;

#else
	gl_FragColor = vec4( 1.0, 1.0, 0.5, 1.0 );
#endif

}
