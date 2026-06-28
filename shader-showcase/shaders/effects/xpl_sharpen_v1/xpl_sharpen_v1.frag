#version 460
// Sharpen V1
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec2 ps=1.0/uResolution;vec2 hp=ps*0.5;vec4 blur=texture(uInputTex,vUV+vec2(hp.x,-ps.y));blur+=texture(uInputTex,vUV+vec2(-ps.x,-hp.y));blur+=texture(uInputTex,vUV+vec2(ps.x,hp.y));blur+=texture(uInputTex,vUV+vec2(-hp.x,ps.y));blur*=0.25;vec4 sc=texture(uInputTex,vUV);vec4 sharp=sc-blur;float luma=dot(vec4(0.222,0.707,0.071,0),sharp)*uParamFloat0;sc.rgb+=clamp(vec3(luma),-uParamFloat1,uParamFloat1);outColor=sc;
}