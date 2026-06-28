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
    // MatCap UV from normal in view space
    vec2 matcapUV = N.xy * 0.5 + 0.5;
    vec4 matcap = texture(uInputTex, matcapUV);
    float NdotL = max(dot(N, L), 0.0);
    float fresnel = pow(1.0 - abs(dot(N, V)), 3.0);
    // Base color from texture, metallic reflection from matcap
    vec4 tex = texture(uInputTex, vec2(NdotL, 0.5));
    vec3 base = tex.rgb * 0.3 + matcap.rgb * 0.7;
    outColor = vec4(base * uLightColor * (NdotL*0.4+0.6) + vec3(1)*fresnel*0.2, 1.0);
}
