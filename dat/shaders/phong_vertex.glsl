#version 110
// Phong Vertex Shader

// Attributes
attribute vec4 position;
attribute vec4 normal;

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;
varying vec4 projected_frag_position;
varying vec4 test_color;

// Uniform
uniform	mat4 projection;
uniform	mat4 modelview;

void main() {
	gl_Position = projection * modelview * position;
	test_color = vec4( gl_Position.z * 0.5, 0.0, 0.0, 1.0 );
///	gl_Position.z = 20.0;
//	gl_Position.w = 1.0;
	frag_position = modelview * position;
	projected_frag_position = projection * modelview * position;
	frag_normal = modelview * normal;
}
