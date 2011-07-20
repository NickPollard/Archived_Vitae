#version 110
// Phong Fragment Shader

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;

void main() {
	vec4 light_specular = vec4( 0.5, 0.5, 0.5, 1.0 );
	vec4 light_diffuse = vec4( 1.0, 1.0, 1.0, 1.0 );

	// For now, test with light at origin
	vec4 light_direction = normalize( frag_position );
	float diffuse = max( 0.0, dot( -light_direction, frag_normal ));
	vec4 diffuse_color = light_diffuse * diffuse;

	vec4 spec_bounce = reflect( light_direction, frag_normal );
	vec4 view_direction = normalize( frag_position );
	float spec = max( 0.0, dot( spec_bounce, -view_direction ));
	float shininess = 3.0;
	float specular = pow( spec, shininess );
	vec4 specular_color = light_specular * specular;

	gl_FragColor = specular_color + diffuse_color;
	
}
