#ifdef GL_ES
precision highp float;
#endif
//#version 110
//#define NORMAL_MAPPING

// Varying
varying vec4 frag_position;
varying vec4 cameraSpace_frag_normal;
varying vec4 frag_normal;
varying vec2 texcoord;
varying vec2 cliff_texcoord;
varying vec4 vert_color;
varying float fog;
varying vec4 local_fog_color;
varying float cliff;
#ifdef NORMAL_MAPPING
#else
varying float specular;
#endif // NORMAL_MAPPING

// Uniform
uniform sampler2D tex;
uniform sampler2D tex_b;
uniform sampler2D tex_c;
uniform sampler2D tex_d;

uniform sampler2D tex_normal;
uniform sampler2D tex_b_normal;

uniform sampler2D tex_lookup;
uniform vec4 directional_light_direction;

uniform mat4 modelview;

// Test Light values
vec4 light_ambient = vec4( 0.2, 0.2, 0.3, 1.0 );
// Directional Light
vec4 directional_light_diffuse = vec4( 1.0, 1.0, 0.8, 1.0 );
vec4 directional_light_specular = vec4( 0.4, 0.4, 0.4, 1.0 );

float diffuse_warp( float diffuse ) {
	//return diffuse * 0.5 + 0.5;
	return diffuse;
}

void main() {
#if 1
	// light-invariant calculations
	vec4 total_light_color = light_ambient;
	vec4 view_direction = normalize( frag_position );

#ifdef NORMAL_MAPPING
	vec4 texture_normal = texture2D( tex_normal, texcoord );
	vec4 texture_b_normal = texture2D( tex_b_normal, texcoord );
	float x = texture_normal.x - 0.5 * 2.0; 
	float z = texture_normal.z - 0.5 * 2.0; 
	float y = 1.0 - ( x*x + z*z );
	vec4 image_normal = vec4( x, y, z, 0.0 );
	x = texture_b_normal.x - 0.5 * 2.0; 
	z = texture_b_normal.z - 0.5 * 2.0; 
	y = 1.0 - ( x*x + z*z );
	vec4 image_b_normal = vec4( x, y, z, 0.0 );
	vec4 imagespace_normal = normalize( mix( image_normal, image_b_normal, cliff ));

	vec4 binormal = vec4( 0.0, 1.0, 0.0, 0.0 );
	vec4 tangent = vec4( cross( binormal.xyz, frag_normal.xyz ), 0.0 );
	mat4 tangent_space = mat4( tangent, binormal, frag_normal, vec4( 0.0, 0.0, 0.0, 1.0 ) );
	vec4 normal = modelview * tangent_space * imagespace_normal;
#else
	 vec4 normal = cameraSpace_frag_normal;
#endif

	// Directional light
	{
		// Ambient + Diffuse
		// TODO: Can this be done in the vertex shader?
		// how does dot( -light_direction, normal ) vary accross a poly?
		float diffuse = max( 0.0, dot( -directional_light_direction, normal ));
		total_light_color += directional_light_diffuse * diffuse_warp( diffuse );
		
		//total_light_color += directional_light_specular * pow( specular, shininess );
#ifdef NORMAL_MAPPING
		vec4 spec_bounce = reflect( directional_light_direction, normal );
		float specular = max( 0.0, dot( spec_bounce, -view_direction ));
#endif // NORMAL_MAPPING
		//float shininess = 1.0;
		// Whilst shininess == 1.0, don't do any pow()
		total_light_color += directional_light_specular * specular;
	}
	vec4 ground_color = texture2D( tex, texcoord );
	vec4 cliff_color = texture2D( tex_b, cliff_texcoord );
	vec4 tex_color = mix( ground_color, cliff_color, cliff );

	vec4 ground_color_2 = texture2D( tex_c, texcoord );
	vec4 cliff_color_2 = texture2D( tex_d, cliff_texcoord );
	vec4 tex_color_2 = mix( ground_color_2, cliff_color_2, cliff );

	vec4 grey = vec4( 0.5, 0.5, 0.5, 1.0 );
	vec4 black = vec4( 0.0, 0.0, 0.0, 1.0 );

	vec4 fragColor = total_light_color * mix( tex_color, tex_color_2, vert_color.x );
	//vec4 fragColor = total_light_color * grey;

	gl_FragColor = mix( fragColor, local_fog_color, fog );
	gl_FragColor.w = 1.0;

#else
	gl_FragColor = vec4( 1.0, 1.0, 0.5, 1.0 );
#endif

}
