#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm;


vec2 positions[3] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(0.0, 0.5)
);
vec3 color[4] = vec3[](
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1),
    vec3(1,0,1)
);

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    mat4 model;
    float Time;
} ubo;

layout(location = 0) out vec3 out_Pos;
layout(location = 1) out vec2 out_uv;

void main() 
{
    out_uv = vec2(in_uv.x, 1.0 - in_uv.y);
    out_Pos = color[gl_VertexIndex] * cos(ubo.Time) * 0.5 + 0.5;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_pos, 1.0);
}