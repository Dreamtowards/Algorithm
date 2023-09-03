#version 450

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
    gl_Position = vec4(positions[gl_VertexIndex] * 2, 0.0, 1.0);
}