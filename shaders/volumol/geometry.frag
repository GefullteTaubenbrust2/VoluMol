#version 330 core

layout(location = 0) out vec4 FragPosition;
layout(location = 1) out vec4 FragNormal;

in vec3 normal;
in vec3 position;

void main() {
	FragPosition = vec4(position, 1.0);
	FragNormal = vec4(normal, 1.0);
}