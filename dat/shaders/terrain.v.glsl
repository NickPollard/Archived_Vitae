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
varying vec4 cameraSpace_frag_normal;
varying vec2 texcoord;
varying vec4 vert_color;
varying float fog;
varying float cliff;

// Uniform
uniform	mat4 projection;
uniform	mat4 modelview;

void main() {
	gl_Position = projection * modelview * position;
#if 1
	frag_position = modelview * position;
	cameraSpace_frag_normal = modelview * normal;
	texcoord = uv.xy;

	//vert_color = vec4( 0.8, 0.9, 1.0, 1.0 );
	vert_color = vec4( color.xyz, 1.0 );
	cliff = color.w;

	// We can calculate fog per vertex as we know polys will be small for terrain
	float height = position.y;
	float fog_far = 350.0;
	float fog_near = 100.0;
	float fog_height = 160.0;
	float height_factor = clamp( ( fog_height - height ) / fog_height, 0.0, 1.0 );
	float max_distance = 350.0;
	float distance = min( max_distance, frag_position.z );
	float fog_max = 0.4;
	fog = clamp( ( distance - fog_near ) / ( fog_far - fog_near ), 0.0, fog_max ) * height_factor;
#endif
}
