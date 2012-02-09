//#version 110
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
varying vec2 texcoord;
varying vec4 vert_color;
varying float fog;

// Uniform
uniform	mat4 projection;
uniform	mat4 modelview;

void main() {
	gl_Position = projection * modelview * position;
#if 1
	frag_position = modelview * position;
	frag_normal = modelview * normal;
	texcoord = uv.xy;
	float height = position.y;

	float r = clamp( height / 10.0 + 0.2, 0.4, 0.8 );
	float g = clamp( height / 20.0, 0.2, 0.3 );
	float b = clamp( height / 20.0, 0.2, 0.3 );
	vert_color = vec4(r, g, b, 1.0 );

	float fog_far = 150.0;
	float fog_near = 50.0;
	fog = ( frag_position.z - fog_near ) / ( fog_far - fog_near );
#endif
}
