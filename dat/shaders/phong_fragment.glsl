#version 110
// Phong Fragment Shader

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;

// Uniform
uniform mat4 modelview;
uniform vec4 light_position;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

// Test Light values
const vec4 light_ambient = vec4( 0.2, 0.2, 0.2, 0.0 );

const vec4 material_diffuse = vec4( 1.0, 1.0, 1.0, 1.0 );
const vec4 material_specular = vec4( 1.0, 1.0, 1.0, 1.0 );

void main() {

	vec4 cs_light_position = modelview * light_position;
	vec4 light_direction = normalize( frag_position - cs_light_position );

	// Ambient + Diffuse
	float diffuse = max( 0.0, dot( -light_direction, frag_normal ));
	vec4 diffuse_color = light_diffuse * diffuse + light_ambient;

	// Specular
	vec4 spec_bounce = reflect( light_direction, frag_normal );
	vec4 view_direction = normalize( frag_position );
	float spec = max( 0.0, dot( spec_bounce, -view_direction ));
	float shininess = 3.0;
	float specular = pow( spec, shininess );
	vec4 specular_color = light_specular * specular;

	gl_FragColor = specular_color * material_specular + diffuse_color * material_diffuse;
	
}
