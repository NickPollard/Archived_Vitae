#version 110
// Phong Fragment Shader

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;

void main() {
	gl_FragColor = vec4( frag_normal.x, frag_normal.y, frag_normal.z, 1.0 );
}
