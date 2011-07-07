#version 110
// Phong Vertex Shader

// Attributes
attribute vec4 position;

// Varying
varying vec4 pos;

// Uniform
uniform	mat4 modelview;
uniform	mat4 projection;

void main() {
	gl_Position = position;
	pos = position;
//	gl_Position = projection * modelview * position;
	float scale = 0.5;
	gl_Position = vec4( gl_Position.x * scale, gl_Position.y * scale, 0.f, 1.f);
}
