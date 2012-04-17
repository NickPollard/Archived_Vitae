//#version 110
#ifdef GL_ES
precision mediump float;
#endif
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
//varying float steepness;

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

	vert_color = vec4( 0.1, 0.1, 0.1, 1.0 );

	/*
	float incline = clamp( 1.0 - dot( normal, vec4( 0.0, 1.0, 0.0, 0.0 )), 0.0, 1.0 );
	const float halfpi = 3.1415926 / 2.0;
	steepness = clamp((asin( incline ) / halfpi) * 4.0, 0.0, 1.0);

	*/
	float fog_far = 350.0;
	float fog_near = 100.0;
	float fog_height = 160.0;
	float height_factor = clamp( ( fog_height - height ) / fog_height, 0.0, 1.0 );
	fog = clamp( ( frag_position.z - fog_near ) / ( fog_far - fog_near ), 0.0, 1.0 ) * height_factor;
#endif
}
