#ifdef GL_ES
precision mediump float;
#endif
//#version 110

// Varying
varying vec4 frag_position;
varying vec4 cameraSpace_frag_normal;
varying vec2 texcoord;
varying vec4 vert_color;
varying float fog;

const int LIGHT_COUNT = 2;

// Uniform
uniform mat4 modelview;
//uniform vec4 light_position[LIGHT_COUNT];
//uniform vec4 light_diffuse[LIGHT_COUNT];
//uniform vec4 light_specular[LIGHT_COUNT];
uniform sampler2D tex;
uniform vec4 fog_color;
uniform vec4 camera_space_sun_direction;

// Test Light values
const vec4 light_ambient = vec4( 0.2, 0.2, 0.2, 1.0 );
// Directional Light
const vec4 directional_light_direction = vec4( 1.0, -0.5, 1.0, 0.0 );
const vec4 directional_light_diffuse = vec4( 1.0, 1.0, 0.4, 1.0 );
const vec4 directional_light_specular = vec4( 0.5, 0.5, 0.5, 1.0 );

const vec4 sun_color = vec4( 1.0, 0.5, 0.0, 0.0 );

const float light_radius = 20.0;

float sun_fog( vec4 local_sun_dir, vec4 fragment_position ) {
	return max( 0.0, dot( local_sun_dir, normalize( fragment_position )));
}

void main() {
#if 1
	// light-invariant calculations
	vec4 view_direction = normalize( frag_position );

	vec4 total_diffuse_color = light_ambient;
	vec4 total_specular_color = vec4( 0.0, 0.0, 0.0, 0.0 );

#if 1
	// Directional light
	{
		// Ambient + Diffuse
		// TODO: This is constant, can be calculated once
		vec4 light_direction = normalize( modelview * directional_light_direction );

		// TODO: Can this be done in the vertex shader?
		// how does dot( -light_direction, cameraSpace_frag_normal ) vary accross a poly?
		float diffuse = max( 0.0, dot( -light_direction, cameraSpace_frag_normal ));
		vec4 diffuse_color = directional_light_diffuse * diffuse;
		total_diffuse_color += diffuse_color;
		
		// Specular
		vec4 spec_bounce = reflect( light_direction, cameraSpace_frag_normal );
		float spec = max( 0.0, dot( spec_bounce, -view_direction ));
		float shininess = 1.0;
		float specular = pow( spec, shininess );
		vec4 specular_color = directional_light_specular * specular;
		total_specular_color += specular_color;
	}
#endif
#define ENABLE_POINT_LIGHTS 0
#if ENABLE_POINT_LIGHTS	
	for ( int i = 0; i < LIGHT_COUNT; i++ ) 
		// Per-light calculations
		vec4 cs_light_position = modelview * light_position[i];
		vec4 light_direction = normalize( frag_position - cs_light_position );
		float light_distance = length( frag_position - cs_light_position );

		// Ambient + Diffuse
		float diffuse = max( 0.0, dot( -light_direction, cameraSpace_frag_normal )) * max( 0.0, ( light_radius - light_distance ) / ( light_distance - 0.0 ) );
		vec4 diffuse_color = light_diffuse[i] * diffuse;
		total_diffuse_color += diffuse_color;

		// Specular
		vec4 spec_bounce = reflect( light_direction, cameraSpace_frag_normal );
		float spec = max( 0.0, dot( spec_bounce, -view_direction ));
		float shininess = 10.0;
		float specular = pow( spec, shininess );
		vec4 specular_color = light_specular[i] * specular;
		total_specular_color += specular_color;
	}
#endif // ENABLE_POINT_LIGHTS

	vec4 tex_color = texture2D( tex, texcoord );
	vec4 material_diffuse = vert_color * tex_color;
	vec4 fragColor = (/*total_specular_color +*/ total_diffuse_color) * material_diffuse;

	// sunlight on fog
	float fog_sun_factor = sun_fog( camera_space_sun_direction, frag_position );
	//vec4 local_fog_color = mix( fog_color, sun_color, fog_sun_factor );
	vec4 local_fog_color = fog_color + (sun_color * fog_sun_factor);

	gl_FragColor = mix( fragColor, local_fog_color, fog );
	//gl_FragColor = fragColor;
	gl_FragColor.w = 1.0;

#else
	gl_FragColor = vec4( 1.0, 1.0, 0.5, 1.0 );
#endif

}
