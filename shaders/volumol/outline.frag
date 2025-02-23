#version 330

in vec4 color;
in vec2 texCoord;

uniform sampler2D depth_tex;
uniform float z_near;
uniform float z_far;

out vec4 FragColor;

float linearize_depth(float depth) {
#ifdef ORTHOGRAPHIC
	return (z_far - z_near) * depth + z_near;
#else
	return z_near * z_far / (z_far + depth * (z_near - z_far));
#endif
}

float smoothStep(float x0, float x1, float x) {
	float y = (x - x0) / (x1 - x0);
	return clamp(y, 0.0, 1.0);
}

void main() {
	vec2 texel_size = 1.0 / textureSize(depth_tex, 0);

	float d00 = linearize_depth(texture2D(depth_tex, texCoord).r);
	float d01 = linearize_depth(texture2D(depth_tex, texCoord + vec2(-0.5,  0.0) * texel_size).r);
	float d02 = linearize_depth(texture2D(depth_tex, texCoord + vec2( 0.5,  0.0) * texel_size).r);
	float d10 = linearize_depth(texture2D(depth_tex, texCoord + vec2( 0.0, -0.5) * texel_size).r);
	float d20 = linearize_depth(texture2D(depth_tex, texCoord + vec2( 0.0,  0.5) * texel_size).r);

	float laplace = 4.0 * d00 - d01 - d02 - d10 - d20;

	gl_FragColor = vec4(vec3(smoothstep(0.04, 0.08, abs(laplace))), 1.0);
}
