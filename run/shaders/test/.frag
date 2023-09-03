#version 450

layout(location = 0) in vec3 _Pos;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = vec4(_Pos, 1.0);

    outColor.rgb = texture(texSampler, in_uv).rgb;
}