#version 110
// Phong Vertex Shader

// Attributes
attribute vec4 position;
attribute vec4 normal;

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;

// Uniform
uniform	mat4 modelview;
uniform	mat4 projection;
uniform vec4 light_position;

void main() {
	gl_Position = projection * modelview * position;
	frag_position = modelview * position;
	frag_normal = modelview * normal;
}
