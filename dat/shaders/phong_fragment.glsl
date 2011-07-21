#version 110
// Phong Fragment Shader

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;

// Uniform
uniform mat4 worldspace;
uniform vec4 light_position;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

// Test Light values
const vec4 light_ambient = vec4( 0.2, 0.2, 0.2, 0.0 );

const vec4 material_diffuse = vec4( 1.0, 1.0, 1.0, 1.0 );
const vec4 material_specular = vec4( 0.0, 0.0, 1.0, 1.0 );
const float light_radius = 10.0;

void main() {

	// light-invariant calculations
	vec4 view_direction = normalize( frag_position );

	// Per-light calculations
	vec4 cs_light_position = worldspace * light_position;
	vec4 light_direction = normalize( frag_position - cs_light_position );
	float light_distance = length( frag_position - cs_light_position );

	// Ambient + Diffuse
	float diffuse = max( 0.0, dot( -light_direction, frag_normal )) * max( 0.0, ( light_radius - light_distance ) / ( light_distance - 0.0 ) );
	vec4 diffuse_color = light_diffuse * diffuse;

	// Specular
	vec4 spec_bounce = reflect( light_direction, frag_normal );
	float spec = max( 0.0, dot( spec_bounce, -view_direction ));
	float shininess = 3.0;
	float specular = pow( spec, shininess );
	vec4 specular_color = light_specular * specular;

	vec4 total_diffuse_color = diffuse_color + light_ambient;
	gl_FragColor =	specular_color * material_specular + 
					total_diffuse_color * material_diffuse;
	
}
