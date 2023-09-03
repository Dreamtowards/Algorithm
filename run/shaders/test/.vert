#version 450

vec2 positions[3] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(0.0, 0.5)
);

layout(location = 0) out vec3 out_Pos;

void main() {
    out_Pos = vec3(positions[gl_VertexIndex], 1);
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}