#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(binding = 1, std140) uniform Params {
    float uP0; float uP1; float uP2; float uP3;
    float uP4; float uP5; float uP6; float uP7;  // P6-P7 = resolution
    float uTime; float uFrameCount; float uPad0; float uPad1;
    mat4 uMVP;         // offset 48
    mat4 uModelView;   // offset 112
    vec4 uLightDir;    // offset 176
    vec4 uLightColor;  // offset 192
    vec4 uEyePos;      // offset 208
};

layout(location = 0) out vec3 vViewPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vUV;

void main() {
    vec4 viewPos = uModelView * vec4(aPos, 1.0);
    vViewPos = viewPos.xyz;
    vNormal = mat3(uModelView) * aNormal;
    vUV = aUV;
    gl_Position = uMVP * vec4(aPos, 1.0);
}