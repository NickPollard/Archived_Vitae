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
varying vec2 texcoord;
varying float fog;
varying vec4 frag_position;
//varying float height;

// Uniform
uniform	mat4 projection;
uniform	mat4 modelview;

void main() {
	gl_Position = projection * modelview * position;
	frag_position = modelview * position;
	texcoord = uv.xy;

	// height is relative to the world, not to the camera
	float height = position.y / 500.0;
	// distance is camera relative
	vec4 frag_position = modelview * position;

	//vec4 fog_color = vec4( 1.0, 0.6, 0.2, 1.0 );
	//float fog_far = 350.0;
	//float fog_near = 100.0;
	float fog_height = 260.0;
	//float max_distance = 250.0;
	//float distance = min( max_distance, frag_position.z );
	float distance = 500.0;

	float height_factor = clamp( ( fog_height - (height * 500.0)) / fog_height, 0.0, 1.0 );
	// sky is always maximum distance away
	fog = height_factor;
	//fog = clamp( ( distance - fog_near ) / ( fog_far - fog_near ), 0.0, 1.0 ) * height_factor;


}
