#version 330

in vec3 ray_direction;
in vec3 ray_view;
in vec2 screenspace_position;
in vec3 ray_origin;

out vec4 FragColor;

uniform sampler3D cubemap;
uniform sampler2D depth_map;
uniform sampler2D background;

uniform vec3 camera_position;
uniform vec3 cubemap_origin;
uniform vec3 cubemap_size;
uniform vec3 sun_direction;
uniform vec3 sun_color;
uniform vec3 ambient_color;

uniform int iterations;
uniform int light_iterations;
uniform float light_distance;

uniform vec3 positive_color;
uniform vec3 negative_color;

uniform float z_near;
uniform float z_far;

uniform sampler2DArray shadow_map;
uniform int levels;
uniform mat4[8] light_matrices;
uniform float[8] layer_depths;

uniform float density_factor;
uniform float density_cutoff;

uniform bool orthographic;

vec3 light_step = sun_direction * light_distance / float(light_iterations);

#define PI 3.141592653589

// From http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// and https://www.shadertoy.com/view/MslGR8 (5th one starting from the bottom)
// NOTE: `frag_coord` is in pixels (i.e. not normalized UV).
float screen_space_dither(highp vec2 uv) {
	// Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR.
	float dither = dot(vec2(171.0, 231.0), uv);
	dither = fract(dither / 103.0);

	// Subtract 0.5 to avoid slightly brightening the whole viewport.
	return dither;
}

// custom smoothstep implementation because it's not defined in glsl1.2
// https://docs.gl/sl4/smoothstep
float smoothStep(in float edge0, in float edge1, in float x)
{
	float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
	return t * t * (3.0 - 2.0 * t);
}

float getPsi(vec3 p) {
	p -= cubemap_origin;
	p /= cubemap_size;
	return texture3D(cubemap, p).x;
}

float sampleShadowMap(int level, vec3 pos) {
	vec4 light_pos = light_matrices[level] * vec4(pos, 1.0);
	light_pos.xyz /= light_pos.w;
	light_pos = light_pos * 0.5 + 0.5;
	float depth = texture(shadow_map, vec3(light_pos.xy, float(level))).r;
	return depth > light_pos.z ? 1.0 : 0.0;
}

float getBrightness(vec3 pos) {
	float density = 0.0;
	for (int i = 1; i <= light_iterations; i++) {
		vec3 p = pos + sun_direction * float(i) / float(light_iterations) * light_distance;
		density += abs(getPsi(p)) / float(light_iterations) * light_distance;
	}
	return exp(-density_factor * density);
}

vec3 blend3D(vec3 A, vec3 B, float alphaA, float alphaB) {
    float alphaC = alphaA + (1. - alphaA) * alphaB;
    return (alphaA * A + (1. - alphaA) * alphaB * B) / max(alphaC, 0.00000001);
}

float blendAlpha(float alphaA, float alphaB) {
	return alphaA + (1. - alphaA) * alphaB;
}

vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) {
	vec3 t0 = (boundsMin - rayOrigin) * invRaydir;
	vec3 t1 = (boundsMax - rayOrigin) * invRaydir;
	vec3 tmin = min(t0, t1);
	vec3 tmax = max(t0, t1);
                
	float dstA = max(max(tmin.x, tmin.y), tmin.z);
	float dstB = min(tmax.x, min(tmax.y, tmax.z));

	float dstToBox = max(0, dstA);
	float dstInsideBox = max(0, dstB - dstToBox);
	return vec2(dstToBox, dstToBox + dstInsideBox);
}

float toLinearDepth(float depth)  {
	if (orthographic) return (z_far - z_near) * depth + z_near;
	else return z_near * z_far / (z_far + depth * (z_near - z_far));
}

void main() {
	vec3 view_vec = normalize(ray_direction);

	vec2 ray_params = rayBoxDst(cubemap_origin, cubemap_origin + cubemap_size, camera_position + ray_origin, 1.0 / view_vec);

	float depth = toLinearDepth(texture2D(depth_map, screenspace_position * 0.5 + 0.5).r) / -normalize(ray_view).z;

	float start_depth = min(ray_params.x, depth);
	float end_depth = min(ray_params.y, depth);
	float dx = (end_depth - start_depth) / float(iterations);

	float bias = screen_space_dither(gl_FragCoord.xy);

	float out_scatter_strength = pow(dot(sun_direction, view_vec) * 0.5 + 0.5, 16.0);

	float transmission = 1.0;
	vec3 color = vec3(0.0);
	for (int i = 0; i < iterations; i++) {
		vec3 p = camera_position + ray_origin + view_vec * (start_depth + (float(i) + bias) / float(iterations) * (end_depth - start_depth));

		float psi = getPsi(p);
		float rho = abs(psi);

		if (rho < density_cutoff) continue;

		float local_density = density_factor * rho * dx;

		float brightness = getBrightness(p);
		brightness *= (1.0 - exp(-rho * density_factor)) * (1.0 - out_scatter_strength) + out_scatter_strength;
		vec3 local_color = (psi > 0.0 ? positive_color : negative_color) * (ambient_color + sun_color * brightness);

		float local_transmission = exp(-local_density);

		color = blend3D(color, local_color, 1.0 - transmission, 1.0 - local_transmission);

		transmission *= local_transmission;

		if (transmission < 0.0001) break;
	}

	vec4 src = texture2D(background, screenspace_position * 0.5 + 0.5);

	FragColor = max(vec4(0.0), vec4(blend3D(color, src.rgb, 1.0 - transmission, src.a), blendAlpha(1.0 - transmission, src.a)));
}
