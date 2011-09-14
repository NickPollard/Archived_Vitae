//#version 110
// Phong Fragment Shader

#ifdef GL_ES
precision mediump float;
#endif

// Varying
varying vec4 frag_position;
varying vec2 texcoord;
const int LIGHT_COUNT = 2;

// Uniform
uniform mat4 worldspace;
uniform vec4 light_position[LIGHT_COUNT];
uniform vec4 light_diffuse[LIGHT_COUNT];
uniform vec4 light_specular[LIGHT_COUNT];
uniform sampler2D tex;

// Test Light values
const vec4 light_ambient = vec4( 0.2, 0.2, 0.2, 0.0 );
// Directional Light
const vec4 directional_light_direction = vec4( 1.0, -1.0, 1.0, 0.0 );
const vec4 directional_light_diffuse = vec4( 0.2, 0.2, 0.1, 1.0 );
const vec4 directional_light_specular = vec4( 0.2, 0.2, 0.1, 1.0 );

const vec4 material_specular = vec4( 0.5, 0.5, 0.5, 1.0 );
const float light_radius = 20.0;

void main() {
	// light-invariant calculations
	vec4 view_direction = normalize( frag_position );

	vec4 total_diffuse_color = light_ambient;
	vec4 total_specular_color = vec4( 0.0, 0.0, 0.0, 0.0 );

	vec4 material_diffuse = texture2D( tex, texcoord );
	gl_FragColor =	total_specular_color * material_specular + 
					total_diffuse_color * material_diffuse;
	gl_FragColor.w = 1.0;

	const float distance = 100.0;
	float height = ( frag_position.y + distance ) / ( 2.0 * distance );

	gl_FragColor = vec4(	clamp( 4.0 - (height * 6.0) , 0.0, 1.0 ),
		   					clamp( (4.0 - (height * 6.0)) * 0.5, 0.0, 1.0 ), 
//							clamp( 0.6 - (height/1.4), 0.0, 0.1 ), 
							clamp( min( height*2.0 - 1.0, 1.0 - height ), 0.0, 1.0 ),
							1.0 );
//	gl_FragColor = frag_position;
	gl_FragColor = material_diffuse;

}
