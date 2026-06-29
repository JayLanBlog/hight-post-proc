#version 460
// mesh3d.vert — 3D object vertex shader for AUS port (vertex input mode)
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

layout(location=0) out vec3 vWorldPos;
layout(location=1) out vec3 vWorldNormal;
layout(location=2) out vec2 vUV;
layout(location=3) out vec3 vEyeDir;
layout(location=4) out vec3 vLightDir;

layout(std140, binding=1) uniform Params {
    float uParamFloat0, uParamFloat1, uParamFloat2, uParamFloat3, uParamFloat4, uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
    mat4 uMVP;
    mat4 uModelView;
    vec3 uLightDir;
    float _pad0;
    vec3 uLightColor;
    float _pad1;
    vec3 uEyePos;
    float _pad2;
};

void main() {
    vec4 worldPos = vec4(aPos, 1.0);
    gl_Position = uMVP * worldPos;
    vWorldPos    = aPos;
    vWorldNormal = normalize(aNormal);
    vUV          = aUV;
    vEyeDir      = normalize(uEyePos - aPos);
    vLightDir    = normalize(uLightDir);
}
