#version 110
// Phong Fragment Shader

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;

void main() {
	vec4 normal = vec4( 0.0, 0.0, 0.0, 0.0);
	normal.x = abs( frag_normal.x );
	normal.y = abs( frag_normal.y );
	normal.z = abs( frag_normal.z );
	gl_FragColor = vec4( normal.x, normal.y, normal.z, 1.0 );
}
