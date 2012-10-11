// UI fragment shader

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 texcoord;
varying vec4 ui_color;

uniform sampler2D tex;

void main() {
	gl_FragColor = texture2D( tex, texcoord ) * ui_color;
}
