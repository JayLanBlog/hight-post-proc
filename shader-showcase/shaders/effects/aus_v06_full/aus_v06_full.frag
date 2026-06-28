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
    vec3 V = normalize(vEyeDir);
    vec3 L = normalize(vLightDir);
    vec4 bump = texture(uInputTex, vUV);
    vec3 perturb = normalize(N + (bump.rgb - 0.5) * uParamFloat0);
    float NdotL = max(dot(perturb, L), 0.0);
    float rim = 1.0 - abs(dot(perturb, V));
    rim = pow(rim, uParamFloat1);
    vec3 rimColor = vec3(uParamFloat2, uParamFloat3, uParamFloat4);
    vec4 tex = texture(uInputTex, vUV);
    vec4 detail = texture(uInputTex, vUV * uParamFloat5);
    outColor = vec4(tex.rgb * detail.rgb * 1.5 * uLightColor * (NdotL+0.2) + rimColor*rim*0.4, 1.0);
}
