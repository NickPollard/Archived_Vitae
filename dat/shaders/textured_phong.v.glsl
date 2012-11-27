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
varying float fog;

// Uniform
uniform	mat4 projection;
uniform	mat4 modelview;

void main() {
	gl_Position = projection * modelview * position;
	frag_position = modelview * position;
	frag_normal = modelview * normal;
	texcoord = uv.xy;
	frag_color = color;

	// We can calculate fog per vertex as we know polys will be small for terrain
	// TODO Change for this shader, move to fragment?
	float height = position.y;
	float fog_far = 450.0;
	float fog_near = 200.0;
	float fog_height = 160.0;
	float height_factor = clamp( ( fog_height - height ) / fog_height, 0.0, 1.0 );
	float max_distance = 350.0;
	float distance = min( max_distance, frag_position.z );
	float fog_max = 0.4;
	fog = clamp( ( distance - fog_near ) / ( fog_far - fog_near ), 0.0, fog_max ) * height_factor;

}
