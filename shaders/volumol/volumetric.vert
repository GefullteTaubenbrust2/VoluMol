#version 330
layout (location = 0) in vec3 aPos;

out vec3 ray_direction;
out vec3 ray_view;
out vec2 screenspace_position;
out vec3 ray_origin;

uniform mat3 transformations;
uniform mat4 view;
uniform mat4 proj_inv;

uniform vec3 camera_dir;
uniform bool orthographic;

void main() {
	gl_Position = vec4((transformations * vec3(aPos.xy, 1.0)).xy, aPos.z, 1.0);
	screenspace_position = aPos.xy;
	vec4 p = proj_inv * vec4(aPos.xy, 0.0, 1.0);
	p.xyz /= p.w;
	ray_origin = p.xyz * mat3(view);
	if (orthographic) {
		ray_view = vec3(0.0, 0.0, -1.0);
		ray_direction = camera_dir;
	}
	else {
		ray_view = p.xyz;
		ray_direction = normalize(ray_origin);
	}
	ray_origin -= ray_direction * dot(ray_direction, ray_origin);
}
