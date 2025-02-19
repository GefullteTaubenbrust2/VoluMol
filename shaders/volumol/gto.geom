#version 330
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3[3] v_pos;
in int[3] layer;
out vec3 pos;

void main() {
	pos = v_pos[0];
	gl_Layer = layer[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	pos = v_pos[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	pos = v_pos[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	EndPrimitive();
}
