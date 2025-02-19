#version 330
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in int[3] layer;

out vec3 pos;

void main() {
	gl_Layer = layer[0];
	pos = gl_in[0].gl_Position.xyz;
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	pos = gl_in[1].gl_Position.xyz;
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	pos = gl_in[2].gl_Position.xyz;
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	EndPrimitive();
}
