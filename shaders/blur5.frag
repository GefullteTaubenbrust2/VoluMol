#version 330

in vec4 color;

in vec2 texCoord;

uniform sampler2D texture;

uniform vec2 direction;

out vec4 FragColor;

//	0.100346	0.097274	0.088613	0.075856	0.061021	0.046128	0.032768	0.021874	0.013722	0.008089	0.004481
// 0.198596	0.175713	0.121703	0.065984	0.028002	0.0093

void main() {
	FragColor = texture2D(texture, texCoord) * 0.198596 +
	(texture2D(texture, texCoord + direction) + texture2D(texture, texCoord - direction)) * 0.175713 +
	(texture2D(texture, texCoord + direction * 2) + texture2D(texture, texCoord - direction * 2)) * 0.121703 +
	(texture2D(texture, texCoord + direction * 3) + texture2D(texture, texCoord - direction * 3)) * 0.065984 +
	(texture2D(texture, texCoord + direction * 4) + texture2D(texture, texCoord - direction * 4)) * 0.028002 +
	(texture2D(texture, texCoord + direction * 5) + texture2D(texture, texCoord - direction * 5)) * 0.0093;
}