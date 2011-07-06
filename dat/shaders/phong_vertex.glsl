#version 110
// Phong Vertex Shader

// Attributes
attribute vec4 position;

// Varying

// Uniform
uniform	mat4 modelview;
uniform	mat4 projection;

void main() {
//	gl_Position = projection * modelview * position;
	gl_Position = position;
}
