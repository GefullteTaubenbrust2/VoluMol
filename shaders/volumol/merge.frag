#version 330

in vec4 color;

in vec2 texCoord;

uniform sampler2D texture;
uniform sampler2D ssao;
uniform sampler2D outlines;

uniform float ssao_intensity;
uniform float ssao_exponent;

uniform float outline_radius;

out vec4 FragColor;

float smoothStep(float x0, float x1, float x) {
	float y = (x - x0) / (x1 - x0);
	return clamp(y, 0.0, 1.0);
}

void main() {
	vec4 color = texture2D(texture, texCoord);

	color.rgb *= pow(texture2D(ssao, texCoord).r, ssao_exponent);

	float outline = pow(smoothStep(1.0, 0.5, outline_radius * outline_radius * texture2D(outlines, texCoord).r), 2.2);

	color.a = max(color.a, 1.0 - outline);
	color.rgb *= outline;

	gl_FragColor = color;
}