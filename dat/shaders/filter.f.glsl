// filter fragment shader

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 texcoord;

uniform sampler2D tex;

void main() {
	vec2 tc;
	tc.x = ( texcoord.x + 1.0 ) * 0.5;
	tc.y = ( texcoord.y + 1.0 ) * 0.5;

	gl_FragColor = texture2D( tex, tc );
//	gl_FragColor = vec4( tc, 0.0, 1.0 );
}
