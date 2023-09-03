#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm;


vec2 positions[3] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(0.0, 0.5)
);
vec3 color[3] = vec3[](
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1)
);

layout(location = 0) out vec3 out_Pos;

void main() {
    out_Pos = color[gl_VertexIndex];
    gl_Position = vec4(in_pos, 1.0);
}