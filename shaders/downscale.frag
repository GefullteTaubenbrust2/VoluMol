#version 330

in vec4 color;

in vec2 texCoord;

uniform sampler2D texture;

uniform vec2 scale_factor;

out vec4 FragColor;

void main() {
	vec4 col = vec4(0);
	vec2 texel_size = 1.0 / textureSize(texture, 0);
	for (int x = 0; x < scale_factor.x; ++x) {
		vec4 ycol = vec4(0);
		float xp = (x - scale_factor.x * 0.5) * texel_size.x + texCoord.x;
		for (int y = 0; y < scale_factor.y; ++y) {
			float yp = (y - scale_factor.x * 0.5) * texel_size.y + texCoord.y;
			col += texture2D(texture, vec2(xp, yp));
		}
	}
	FragColor = col / (scale_factor.x * scale_factor.y);
}