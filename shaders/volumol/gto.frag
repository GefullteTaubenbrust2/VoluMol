#version 330
out vec4 FragColor;

in vec3 pos;

#define PRIMITIVES 16

uniform vec3[PRIMITIVES] exponents;
uniform float[PRIMITIVES] alpha;
uniform float[PRIMITIVES] coeff;
uniform vec3[PRIMITIVES] origin;

float ipow(float f, int p) {
	float r = 1.0;
	for (int i = 0; i < p; ++i) r *= f;
	return r;
}

void main() {
	float psi = 0.0;
	for (int i = 0; i < PRIMITIVES; ++i) {
		vec3 r = pos - origin[i];
		psi += coeff[i] * ipow(r.x, int(exponents[i].x)) * ipow(r.y, int(exponents[i].y)) * ipow(r.z, int(exponents[i].z)) * exp(-alpha[i] * dot(r, r));
	}
	FragColor = vec4(psi, 0.0, 0.0, 0.0);
}
