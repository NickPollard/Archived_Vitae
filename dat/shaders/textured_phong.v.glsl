#ifdef GL_ES
precision mediump float;
#endif

// Phong Vertex Shader

// Attributes
attribute vec4 position;
attribute vec4 normal;
attribute vec4 uv;
attribute vec4 color;

// Varying
varying vec4 frag_position;
varying vec4 frag_normal;
varying vec4 frag_color;
varying vec2 texcoord;

// Uniform
uniform	mat4 projection;
uniform	mat4 modelview;

void main() {
	gl_Position = projection * modelview * position;
	frag_position = modelview * position;
	frag_normal = modelview * normal;
	texcoord = uv.xy;
	frag_color = color;
}
