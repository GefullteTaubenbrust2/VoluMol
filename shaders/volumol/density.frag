#version 330
out vec4 FragColor;

in vec3 pos;

#define PRIMITIVES 64

uniform sampler3D orbital;
uniform float occupation;

void main() {
	float psi = texture3D(orbital, vec3(0.5, 0.5, 1.0) * pos + vec3(0.5, 0.5, 0.0)).r;
	float rho = occupation * psi * psi;
	FragColor = vec4(rho, 0.0, 0.0, 0.0);
}
