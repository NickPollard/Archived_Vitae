// filter fragment shader

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 texcoord;

uniform sampler2D tex;

// Render window size
float rt_h = 480.0;

// Gaussian constants
float offset[3];
float weight[3];

void main() {
	offset[0] = 0.0;
	offset[1] = 1.3846153846;
	offset[2] = 3.2307692308;
	weight[0] = 0.2270270270;
   	weight[1] =	0.3162162162;
   	weight[2] = 0.0702702703;

	vec2 tc;
	tc.x = ( texcoord.x + 1.0 ) * 0.5;
	tc.y = ( texcoord.y + 1.0 ) * 0.5;

//	vec4 color = texture2D( tex, tc );
//	gl_FragColor = vec4( color.y, color.z, color.x, 1.0 );
//	gl_FragColor = texture2D( tex, tc );

	vec2 uv = tc;
	vec3 color = texture2D( tex, uv ).rgb * weight[0];
    for (int i=1; i<3; i++)
    {
      color += texture2D( tex, uv + vec2(0.0, offset[i])/rt_h).rgb * weight[i];
      color += texture2D( tex, uv - vec2(0.0, offset[i])/rt_h).rgb * weight[i];
    }

	gl_FragColor = vec4( color, 1.0 );
}
