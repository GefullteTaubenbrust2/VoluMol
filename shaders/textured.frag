#version 330

in vec4 color;

in vec2 texCoord;

uniform sampler2D texture;

out vec4 FragColor;

void main() {
	FragColor = color * texture2D(texture, texCoord);
}