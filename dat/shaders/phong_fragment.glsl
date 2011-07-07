#version 110
// Phong Fragment Shader

varying vec4 pos;

void main() {
	gl_FragColor = vec4( pos.x, pos.y, 1.0, 1.0 );
}
