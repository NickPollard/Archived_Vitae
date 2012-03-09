//#version 110
// Phong Fragment Shader

#ifdef GL_ES
precision mediump float;
#endif

// Varying
varying vec4 frag_position;
varying vec2 texcoord;
varying float height;
const int LIGHT_COUNT = 2;

// Uniform
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

	vec4 color_top = vec4( 0.3, 0.6, 1.0, 1.0 );
	vec4 color_mid = vec4( 1.0, 0.4, 0.2, 1.0 );
	vec4 color_bottom = vec4( 1.0, 0.4, 0.0, 1.0 );

	float midpoint = 0.4;

	float top = clamp( ((height - midpoint) / (1.0 - midpoint)), 0.0, 1.0 );
	float bottom = clamp( 1.0 - ((height) / (midpoint)), 0.0, 1.0 );
	//float mid = 1.0 - (top + bottom);

	vec4 sky_color = mix( mix( color_mid, color_bottom, bottom ), 
						mix( color_mid, color_top, top ), 
						height );

	float cloud_blend = material_diffuse.w;
	gl_FragColor = mix( sky_color, material_diffuse, cloud_blend );

	vec4 fragColor = color_top * material_diffuse.z + color_bottom * material_diffuse.x;
	fragColor.w = 1.f;
	
	float fog_far = 350.0;
	float fog_near = 100.0;
	float fog_height = 160.0;
	float height_factor = clamp( ( fog_height - (height * 500.0)) / fog_height, 0.0, 1.0 );
	float fog = clamp( ( frag_position.z - fog_near ) / ( fog_far - fog_near ), 0.f, 1.f ) * height_factor;
	vec4 fog_color = vec4( 1.0, 0.6, 0.2, 1.0 );
	gl_FragColor = mix( fragColor, fog_color, fog );
//	gl_FragColor = vec4( top, bottom, 0.0, 1.0 );
	//gl_FragColor = mix( color_bottom, color_top, height );
//	gl_FragColor = material_diffuse;

}

