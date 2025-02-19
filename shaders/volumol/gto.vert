#version 330
layout (location = 0) in vec3 aPos;

uniform vec4 tint;

uniform mat3 transformations;

out vec3 v_pos;
out int layer;

uniform vec3 cubemap_origin;
uniform vec3 cubemap_size;
uniform int layer_count;

void main() {
	gl_Position = vec4((transformations * vec3(aPos.xy, 1.0)).xy, 0.0, 1.0);
	v_pos = cubemap_origin + (aPos * vec3(0.5, 0.5, 1.0 / float(layer_count)) + vec3(0.5, 0.5, 0.0)) * cubemap_size;
    layer = int(aPos.z);
}
