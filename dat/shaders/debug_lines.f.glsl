//#version 110
// Phong Fragment Shader

#ifdef GL_ES
precision mediump float;
#endif

// Varying
varying vec4 frag_color;

void main() {
	gl_FragColor = frag_color;
}
