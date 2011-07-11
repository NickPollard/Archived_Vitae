#version 110
// Phong Vertex Shader

// Attributes
attribute vec4 position;

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;

// Uniform
uniform	mat4 modelview;
uniform	mat4 projection;

void main() {
	gl_Position = projection * modelview * position;
	frag_position = gl_Position;
}
