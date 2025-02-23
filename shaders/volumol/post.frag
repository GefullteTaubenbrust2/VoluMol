#version 330

in vec4 color;

in vec2 texCoord;

uniform sampler2D texture;

uniform vec3 clear_color;

uniform float taa_alpha;

uniform float brightness;

out vec4 FragColor;

// From http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// and https://www.shadertoy.com/view/MslGR8 (5th one starting from the bottom)
// NOTE: `frag_coord` is in pixels (i.e. not normalized UV).
vec3 screen_space_dither(highp vec2 uv) {
	// Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR.
	highp vec3 dither = vec3(dot(vec2(171.0, 231.0), uv));
	dither.rgb = fract(dither.rgb / vec3(103.0, 71.0, 97.0));

	// Subtract 0.5 to avoid slightly brightening the whole viewport.
	return (dither.rgb - 0.5) / 255.0;
}

void main() {
	vec3 col = vec3(0.0);

	vec4 s = texture2D(texture, texCoord);
	
#if defined(EMISSIVE_VOLUME) || defined(PREMULTIPLY_COLOR)
	col = s.rgb;
#else
	col = s.rgb * s.a;
#endif

	col = col / (col + vec3(1.0));

	col = min(pow(col, vec3(1.0 / 2.2)) * brightness, 1.0);

#if defined(EMISSIVE_VOLUME) || !defined(PREMULTIPLY_COLOR)
	col = col + clear_color * (1.0 - s.a);
#else
	col = col * s.a + clear_color * (1.0 - s.a);
#endif

	col += screen_space_dither(gl_FragCoord.xy);

	gl_FragColor = vec4(col * taa_alpha, taa_alpha);
}
