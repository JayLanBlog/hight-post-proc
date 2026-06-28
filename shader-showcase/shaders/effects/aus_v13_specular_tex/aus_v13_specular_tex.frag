#version 460
layout(location=0) in vec3 vWorldPos;
layout(location=1) in vec3 vWorldNormal;
layout(location=2) in vec2 vUV;
layout(location=3) in vec3 vEyeDir;
layout(location=4) in vec3 vLightDir;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
    mat4 uMVP; mat4 uModelView;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 L = normalize(vLightDir);
    vec3 V = normalize(vEyeDir);
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), uParamFloat0 * 128.0);
    vec4 tex = texture(uInputTex, vUV);
    vec3 diffuse = tex.rgb * uLightColor * NdotL;
    outColor = vec4(diffuse + vec3(1)*spec*uParamFloat1, 1.0);
}
