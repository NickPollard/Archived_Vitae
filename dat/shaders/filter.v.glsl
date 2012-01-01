// filter vertex shader

attribute vec4 position;

// Need these for shader to work, even though they're unused, as we're currently setting them through macros
attribute vec4 uv;
attribute vec4 normal;
attribute vec4 color;

uniform vec2 screen_size;

varying vec2 texcoord;

void main() {
	vec2 screen_position = position.xy;
	gl_Position = vec4( screen_position.xy, 0.0, 1.0 );
	texcoord = screen_position;
}
