#version 330

in vec4 color;

in vec2 texCoord;

uniform sampler2D texture;

uniform vec2 direction;

out vec4 FragColor;

//	0.100346	0.097274	0.088613	0.075856	0.061021	0.046128	0.032768	0.021874	0.013722	0.008089	0.004481
// 0.198596	0.175713	0.121703	0.065984	0.028002	0.0093

void main() {
	FragColor = texture2D(texture, texCoord) * 0.100346 +
	(texture2D(texture, texCoord + direction) + texture2D(texture, texCoord - direction)) * 0.097274 +
	(texture2D(texture, texCoord + direction * 2) + texture2D(texture, texCoord - direction * 2)) * 0.088613 +
	(texture2D(texture, texCoord + direction * 3) + texture2D(texture, texCoord - direction * 3)) * 0.075856 +
	(texture2D(texture, texCoord + direction * 4) + texture2D(texture, texCoord - direction * 4)) * 0.061021 +
	(texture2D(texture, texCoord + direction * 5) + texture2D(texture, texCoord - direction * 5)) * 0.046128 +
	(texture2D(texture, texCoord + direction * 6) + texture2D(texture, texCoord - direction * 6)) * 0.032768 +
	(texture2D(texture, texCoord + direction * 7) + texture2D(texture, texCoord - direction * 7)) * 0.021874 +
	(texture2D(texture, texCoord + direction * 8) + texture2D(texture, texCoord - direction * 8)) * 0.013722 +
	(texture2D(texture, texCoord + direction * 9) + texture2D(texture, texCoord - direction * 9)) * 0.008089 +
	(texture2D(texture, texCoord + direction * 10) + texture2D(texture, texCoord - direction * 10)) * 0.004481;
}