#version 330
layout (location = 0) in vec3 aPos;

out int layer;

uniform vec4 tint;

uniform mat3 transformations;

uniform int layer_count;

void main() {
	gl_Position = vec4((transformations * vec3(aPos.xy, 1.0)).xy, (aPos.z + 0.5) / float(layer_count), 1.0);
    layer = int(aPos.z);
}
