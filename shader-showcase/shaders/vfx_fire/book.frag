#version 450

layout(binding = 0) uniform sampler2D uInputTex;
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
layout(binding = 2) uniform sampler2D uAuxTex;

layout(location = 0) in vec3 vViewPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir.xyz);
    vec3 V = normalize(uEyePos.xyz - vViewPos);
    vec3 H = normalize(L + V);

    vec3 diffuse = texture(uInputTex, vUV).rgb;
    vec3 normalMap = texture(uAuxTex, vUV).rgb * 2.0 - 1.0;
    N = normalize(N + normalMap);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 32.0);

    vec3 ambient = diffuse * 0.15;
    vec3 diff = diffuse * uLightColor.rgb * NdotL * 2.0;
    vec3 specular = uLightColor.rgb * spec * 0.5;

    outColor = vec4(ambient + diff + specular, 1.0);
}