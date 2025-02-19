#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 color;

out vec2 texCoord;

uniform mat3 transformations;

void main() {
	gl_Position = vec4(transformations * vec3(aPos.xy, 1.), 1.0);
	color = aColor;
	texCoord = aTexCoord;
}