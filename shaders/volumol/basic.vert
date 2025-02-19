#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 4) in vec2 aTexCoord;

out vec3 vertColor;
out vec2 texCoord;
out vec3 normal;
out vec3 vertPos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec2 offset;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_Position += vec4(offset, 0.0, 0.0) * gl_Position.w;
	vertColor = aColor;
	vertPos = (model * vec4(aPos, 1.0)).xyz;
	normal = normalize(mat3(model) * aNormal).xyz;
	texCoord = aTexCoord;
}