#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec2 offset;

out vec3 position;
out vec3 normal;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_Position += vec4(offset, 0.0, 0.0) * gl_Position.w;
	position = (view * model * vec4(aPos, 1.0)).xyz;
	normal = normalize(mat3(view) * mat3(model) * aNormal);
}