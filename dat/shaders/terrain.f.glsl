//#version 110
// Phong Fragment Shader

#ifdef GL_ES
precision mediump float;
#endif

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;
varying vec2 texcoord;
varying float height;

const int LIGHT_COUNT = 2;

// Uniform
uniform mat4 worldspace;
uniform vec4 light_position[LIGHT_COUNT];
uniform vec4 light_diffuse[LIGHT_COUNT];
uniform vec4 light_specular[LIGHT_COUNT];
uniform sampler2D tex;

// Test Light values
const vec4 light_ambient = vec4( 0.0, 0.0, 0.0, 0.0 );
// Directional Light
const vec4 directional_light_direction = vec4( 1.0, -0.5, 1.0, 0.0 );
const vec4 directional_light_diffuse = vec4( 1.0, 1.0, 0.4, 1.0 );
const vec4 directional_light_specular = vec4( 0.2, 0.2, 0.1, 1.0 );

//const vec4 material_diffuse = vec4( 1.0, 1.0, 1.0, 1.0 );
const vec4 material_specular = vec4( 0.5, 0.5, 0.5, 1.0 );
const float light_radius = 20.0;

void main() {
#if 1
	// light-invariant calculations
	vec4 view_direction = normalize( frag_position );

	vec4 total_diffuse_color = light_ambient;
	vec4 total_specular_color = vec4( 0.0, 0.0, 0.0, 0.0 );

	// Directional light
	{
		// Ambient + Diffuse
		vec4 light_direction = normalize( worldspace * directional_light_direction );
		float diffuse = max( 0.0, dot( -light_direction, frag_normal )) * 1.0;
		vec4 diffuse_color = directional_light_diffuse * diffuse;
		total_diffuse_color += diffuse_color;

		// Specular
		vec4 spec_bounce = reflect( light_direction, frag_normal );
		float spec = max( 0.0, dot( spec_bounce, -view_direction ));
		float shininess = 10.0;
		float specular = pow( spec, shininess );
		vec4 specular_color = directional_light_specular * specular;
		total_specular_color += specular_color;
	}
#if 0	
	for ( int i = 0; i < LIGHT_COUNT; i++ ) 
		// Per-light calculations
		vec4 cs_light_position = worldspace * light_position[i];
		vec4 light_direction = normalize( frag_position - cs_light_position );
		float light_distance = length( frag_position - cs_light_position );

		// Ambient + Diffuse
		float diffuse = max( 0.0, dot( -light_direction, frag_normal )) * max( 0.0, ( light_radius - light_distance ) / ( light_distance - 0.0 ) );
		vec4 diffuse_color = light_diffuse[i] * diffuse;
		total_diffuse_color += diffuse_color;

		// Specular
		vec4 spec_bounce = reflect( light_direction, frag_normal );
		float spec = max( 0.0, dot( spec_bounce, -view_direction ));
		float shininess = 10.0;
		float specular = pow( spec, shininess );
		vec4 specular_color = light_specular[i] * specular;
		total_specular_color += specular_color;
	}
#endif

	float r = clamp( height / 10.0 + 0.2, 0.4, 0.8 );
//	float r = 0.8;
	float g = clamp( height / 20.0, 0.2, 0.3 );
	float b = clamp( height / 20.0, 0.2, 0.3 );
	vec4 material_diffuse = vec4( r, g, b, 1.0 ) * texture2D( tex, texcoord );
	vec4 fragColor =	total_specular_color * material_specular + 
					total_diffuse_color * material_diffuse;

	float fog_far = 150.0;
	float fog_near = 50.0;
	float fog = (frag_position.z - fog_near) / ( fog_far - fog_near );
	// Temporary Terrain Fog
	gl_FragColor = mix ( fragColor, vec4( 0.0, 0.0, 0.0, 1.0 ), fog );

	gl_FragColor.w = 1.0;

#endif
//	gl_FragColor = vec4( 0.2, 1.0, 0.2, 1.0 );

}