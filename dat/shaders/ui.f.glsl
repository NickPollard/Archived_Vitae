// UI fragment shader

#ifdef GL_ES
precision mediump float;
#endif


varying vec2 texcoord;

uniform sampler2D tex;

void main() {
	gl_FragColor = texture2D( tex, texcoord ) * vec4( 0.0, 0.5, 0.7, 0.5 );
}
