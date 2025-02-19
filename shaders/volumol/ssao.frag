#version 330

in vec4 color;

in vec2 texCoord;

uniform sampler2D positions;
uniform sampler2D normals;

uniform vec3[64] ssao_offsets;
uniform float ssao_radius;
uniform float ssao_exponent;

uniform mat4 projection;

uniform float rotation_offset;

uniform int iterations;

out vec4 FragColor;

#define PI 3.141592653589

// From http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// and https://www.shadertoy.com/view/MslGR8 (5th one starting from the bottom)
// NOTE: `frag_coord` is in pixels (i.e. not normalized UV).
float screen_space_dither(highp vec2 uv) {
	// Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR.
	highp float dither = dot(vec2(171.0, 231.0), uv);
	dither = fract(dither / 103.0);
	return dither;
}

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float SSAO() {
	vec3 normal = texture2D(normals, texCoord).rgb;
	vec4 pos = texture2D(positions, texCoord);

	if (pos.a <= 0.001) return 1.0;

#if 1
	// Old algorithm for "classic" SSAO
	vec3 s0 = normalize(cross(normal, vec3(1.0, 0.0, 0.0)));
	vec3 t0 = normalize(cross(s0, normal));
	float r = 2.0 * PI * (screen_space_dither(gl_FragCoord.xy) + rotation_offset);
	vec3 s = s0 * cos(r) + t0 * sin(r);
	vec3 t = t0 * cos(r) - s0 * sin(r);

	vec2 texelSize = 1.0 / textureSize(positions, 0);
	float ao = 0.;
	for (int i = 0; i < iterations; i++) {
		vec3 o = ssao_offsets[i];
		vec3 p = pos.xyz + ssao_radius * (s * o.x + t * o.y + normal * o.z);
		vec4 T = projection * vec4(p, 1.0);
		T.xyz = T.xyz / T.w * 0.5 + 0.5;
		vec4 P = texture2D(positions, T.xy);

		float f = (P.z - p.z) * P.a;
		ao += (f > 0.001 && f < 2.0) ? 0.0 : 1.0;
	}
	return ao / float(iterations);
#else
	// New algorithm starts with 2D offsets in screenspace and then samples 3D points.
	// It unfortunately doesn't produce the performance/visual quality imrprovements
	// I had hoped for. I'm not actually sure why :(
	float r = 2.0 * PI * (screen_space_dither(gl_FragCoord.xy) + rotation_offset);
	vec4 a0 = projection * vec4(pos.xyz, 1.0);
	vec4 a1 = projection * vec4(pos.xyz + vec3(ssao_radius, 0.0, 0.0), 1.0);
	vec4 a2 = projection * vec4(pos.xyz + vec3(0.0, ssao_radius, 0.0), 1.0);
	a0.xyz /= a0.w;
	vec2 A = vec2(a1.x / a1.w - a0.x, a2.y / a2.w - a0.y) * 0.5;
	vec2 s = vec2( cos(r), sin(r)) * A;
	vec2 t = vec2(-sin(r), cos(r)) * A;

	float ao = 0.;
	for (int i = 0; i < iterations; ++i) {
		vec2 off = ssao_offsets[i];
		vec4 p = texture2D(positions, texCoord + off.x * s + off.y * t);
		vec3 v = p.xyz - pos.xyz;
		ao += (dot(v, normal) < 0.01 || length(v) > 2.0 * ssao_radius || p.a < 0.01) ? 1.0 : 0.0;
	}
	return ao / float(iterations);
#endif
}

void main() {
	gl_FragColor = vec4(vec3(SSAO()), 1.0);
}